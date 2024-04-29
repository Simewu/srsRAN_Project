/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "io_broker_epoll.h"
#include <sys/epoll.h>
#include <unistd.h>

using namespace srsran;

io_broker_epoll::io_broker_epoll(const io_broker_config& config) : logger(srslog::fetch_basic_logger("IO-EPOLL"))
{
  // Init epoll socket
  epoll_fd = unique_fd{::epoll_create1(0)};
  if (not epoll_fd.is_open()) {
    report_fatal_error("IO broker: failed to create epoll file descriptor. error={}", strerror(errno));
  }

  // Register fd and event_handler to handle stops, fd registrations and fd deregistrations.
  ctrl_event_fd = unique_fd{::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)};
  if (not handle_fd_registration(
          ctrl_event_fd.value(), [this]() { handle_enqueued_events(); }, []() {}, nullptr)) {
    report_fatal_error("IO broker: failed to register control event file descriptor. ctrl_event_fd={}",
                       ctrl_event_fd.value());
  }

  // start thread to handle epoll events.
  std::promise<void> p;
  std::future<void>  fut = p.get_future();
  thread                 = unique_thread(config.thread_name, config.thread_prio, [this, &p]() {
    running = true;
    p.set_value();
    thread_loop();
  });

  // Wait for the thread to start before returning.
  fut.wait();
}

io_broker_epoll::~io_broker_epoll()
{
  // Wait for completion.
  if (thread.running()) {
    enqueue_event(control_event{control_event::event_type::close_io_broker, 0, {}, nullptr});
    thread.join();
  }

  // Close epoll control event fd.
  if (not ctrl_event_fd.close()) {
    logger.error("Failed to close control event socket: {}", strerror(errno));
  }

  // Close epoll socket.
  if (not epoll_fd.close()) {
    logger.error("Failed to close io epoll broker file descriptor: {}", strerror(errno));
  }
}

/// Function is executed in a loop until the thread is notify_stop.
void io_broker_epoll::thread_loop()
{
  while (running) {
    // wait for event
    const int32_t      epoll_timeout_ms   = -1;
    const uint32_t     MAX_EVENTS         = 1;
    struct epoll_event events[MAX_EVENTS] = {};
    int                nof_events         = ::epoll_wait(epoll_fd.value(), events, MAX_EVENTS, epoll_timeout_ms);

    // handle event
    if (nof_events == -1) {
      logger.error("epoll_wait(): {}", strerror(errno));
      /// TODO: shall we raise a fatal error here?
      continue;
    }
    if (nof_events == 0) {
      logger.error("epoll time out {} sec expired", epoll_timeout_ms / 1000.0);
      continue;
    }

    for (int i = 0; i < nof_events; ++i) {
      int      fd           = events[i].data.fd;
      uint32_t epoll_events = events[i].events;
      if ((epoll_events & EPOLLERR) || (epoll_events & EPOLLHUP) || (!(epoll_events & EPOLLIN))) {
        // An error or hang up happened on this file descriptor, or the socket is not ready for reading
        // TODO: add notifier for these events and let the subscriber decide on further actions (e.g. unregister fd)
        if (epoll_events & EPOLLHUP) {
          // Note: some container environments hang up stdin (fd=0) in case of non-interactive sessions
          logger.warning("Hang up on file descriptor. fd={} events={:#x}", fd, epoll_events);
        } else if (epoll_events & EPOLLERR) {
          logger.error("Error on file descriptor. fd={} events={:#x}", fd, epoll_events);
        } else {
          logger.error("Unhandled epoll event. fd={} events={:#x}", fd, epoll_events);
        }

        // Unregister the faulty file descriptor from epoll
        bool success = handle_fd_deregistration(fd, nullptr, true);
        if (!success) {
          logger.error("Failed to unregister file descriptor. fd={}", fd);
        }
        break;
      }

      const auto& it = event_handler.find(fd);
      if (it != event_handler.end()) {
        it->second->handle_event(fd, events[i]);
      } else {
        logger.error("Could not find event handler. fd={}", fd);
      }
    }
  }
}

bool io_broker_epoll::enqueue_event(const control_event& event)
{
  // Push of an event. It may malloc.
  event_queue.enqueue(event);

  // trigger epoll event to interrupt possible epoll_wait()
  uint64_t tmp = 1;
  ssize_t  ret = ::write(ctrl_event_fd.value(), &tmp, sizeof(tmp));
  if (ret == -1) {
    logger.error("Error writing to CTRL event_fd");
  }
  return ret >= 0;
}

void io_broker_epoll::handle_enqueued_events()
{
  // Keep popping from the event queue.
  control_event ev;
  while (event_queue.try_dequeue(ev)) {
    // Handle event dequeued.
    switch (ev.type) {
      case control_event::event_type::register_fd:
        // Register new fd and event handler.
        handle_fd_registration(ev.fd, ev.handler, ev.err_handler, ev.completed);
        break;
      case control_event::event_type::deregister_fd:
        // Deregister fd and event handler.
        handle_fd_deregistration(ev.fd, ev.completed, false);
        break;
      case control_event::event_type::close_io_broker:
        // Close io broker.

        // Start by deregistering all existing file descriptors, except control event fd.
        for (const auto& it : event_handler) {
          if (it.first != ctrl_event_fd.value()) {
            handle_fd_deregistration(it.first, nullptr, false);
          }
        }
        event_handler.clear();

        // Clear queue.
        while (event_queue.try_dequeue(ev)) {
          // Do nothing.
        }

        // Set flag to stop thread loop.
        running = false;
        return;

        break;
      default:
        report_fatal_error("Unknown event type {}", (int)ev.type);
    }
  }
}

bool io_broker_epoll::handle_fd_registration(int                     fd,
                                             const recv_callback_t&  handler,
                                             const error_callback_t& err_handler,
                                             std::promise<bool>*     complete_notifier)
{
  if (event_handler.count(fd) > 0) {
    logger.error("epoll_ctl failed for fd={}. Cause: fd already registered", fd);
    if (complete_notifier != nullptr) {
      complete_notifier->set_value(false);
    }
    return false;
  }

  // Add fd to epoll handler.
  struct epoll_event epoll_ev = {};
  epoll_ev.data.fd            = fd;
  epoll_ev.events             = EPOLLIN;
  if (::epoll_ctl(epoll_fd.value(), EPOLL_CTL_ADD, fd, &epoll_ev) == -1) {
    logger.error("epoll_ctl failed for fd={}", fd);
    if (complete_notifier != nullptr) {
      complete_notifier->set_value(false);
    }
    return false;
  }

  // Register the handler of the fd.
  event_handler.insert({fd, std::make_unique<epoll_receive_callback>(handler, err_handler)});
  if (complete_notifier != nullptr) {
    complete_notifier->set_value(true);
  }
  return true;
}

bool io_broker_epoll::handle_fd_deregistration(int fd, std::promise<bool>* complete_notifier, bool is_error)
{
  auto ev_it = event_handler.find(fd);
  if (ev_it == event_handler.end()) {
    // File descriptor not found. It could have been already deregistered.
    logger.debug("File descriptor not found. fd={}", fd);
    if (complete_notifier != nullptr) {
      complete_notifier->set_value(false);
    }
    return false;
  }

  struct epoll_event epoll_ev = {};
  epoll_ev.data.fd            = fd;
  epoll_ev.events             = EPOLLIN;
  if (::epoll_ctl(epoll_fd.value(), EPOLL_CTL_DEL, fd, &epoll_ev) == -1) {
    logger.error("epoll_ctl failed for fd={}", fd);
    if (complete_notifier != nullptr) {
      complete_notifier->set_value(false);
    }
    return false;
  }

  // Move handler out to call error handler only after the event has been removed from the lookup.
  std::unique_ptr<epoll_handler> rem_it = std::move(ev_it->second);

  // Update lookup
  event_handler.erase(ev_it);

  // Call error handler.
  if (is_error) {
    rem_it->handle_error_event(fd, epoll_ev);
  }

  if (complete_notifier != nullptr) {
    complete_notifier->set_value(true);
  }
  return true;
}

/// Adds a new file descriptor to the epoll-handler. The call is thread-safe and new
/// file descriptors can be added while the epoll_wait() is blocking.
io_broker::io_handle io_broker_epoll::register_fd(int fd, recv_callback_t handler, error_callback_t err_handler)
{
  if (fd < 0) {
    logger.error("io_broker_epoll::register_fd: Received an invalid fd={}", fd);
    return io_handle{};
  }
  if (not running.load(std::memory_order_relaxed)) {
    logger.warning("io_broker_epoll::register_fd: io_broker is not running. fd={}", fd);
    return io_handle{};
  }

  if (std::this_thread::get_id() == thread.get_id()) {
    // Registration from within the epoll thread.
    if (handle_fd_registration(fd, handler, err_handler, nullptr)) {
      return io_handle{*this, fd};
    }
    return io_handle{};
  }

  std::promise<bool> p;
  std::future<bool>  fut = p.get_future();

  enqueue_event(control_event{control_event::event_type::register_fd, fd, handler, err_handler, &p});

  // Wait for the registration to complete.
  if (fut.get()) {
    return io_handle{*this, fd};
  }
  return io_handle{};
}

/// \brief Remove fd from epoll handler.
bool io_broker_epoll::unregister_fd(int fd)
{
  if (fd < 0) {
    logger.error("io_broker_epoll::unregister_fd: Received an invalid fd={}", fd);
    return false;
  }
  if (not running.load(std::memory_order_relaxed)) {
    logger.warning("io_broker_epoll::unregister_fd: io_broker is not running. fd={}", fd);
    return false;
  }

  if (std::this_thread::get_id() == thread.get_id()) {
    // Deregistration from within the epoll thread. No need to go through the event queue.
    return handle_fd_deregistration(fd, nullptr, false);
  }

  std::promise<bool> p;
  std::future<bool>  fut = p.get_future();

  if (not enqueue_event(control_event{control_event::event_type::deregister_fd, fd, {}, {}, &p})) {
    return false;
  }

  // Wait for the deregistration to complete.
  return fut.get();
}
