#ifndef BASELOOP_BASELOOP_H
#define BASELOOP_BASELOOP_H

/** Defining members which are probably could be changed */

#define BASE_LOOP_EVENTS_COUNT 5000
#define BASE_LOOP_READABLE_DATA_SIZE 65000

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"

/** End of definitions */


#if defined(__FreeBSD__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
	#define USE_KQUEUE

	#include <sys/socket.h>
	#include <cstdlib>
	#include <cstdio>
	#include "sys/event.h"
#elif defined(__linux__)
	#define USE_EPOLL
	#include <sys/epoll.h>

#endif

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include "list"
#include "mutex"
#include "memory"
#include "vector"
#include "string"

namespace BaseLoop
{

	using ResolveFN = int(*)(int, const struct sockaddr *, socklen_t);

	struct loop_cmd_t
	{
		int cmd = -1;
		void *data = nullptr;

		loop_cmd_t(int cmd, void *data)
		{
			this->cmd = cmd;
			this->data = data;
		}
	};

	struct loop_event_data_t
	{
		// socket handler
		int fd = -1;

		// public data for outside usage
		void *data = nullptr;
	};

	class EventLoop
	{
	public:
		/// Just an empty destructor/constructor
		~EventLoop() {}
		EventLoop() {}

	private:
		/// just raw number to make sure if we don't have server listener
		/// making sure that event loop would't compare it as a socket number
		int tcp_listener = -1;

		/// member for keeping pipe handle which would be used for sending commands
		/// from other threads to this loop (similar to Thread Channels, but it's NOT!)
		int pipe_chan = -1;
		int event_fd = -1;

		/// Because we are actually keeping command's List as a Queue
		/// we need some locking mutex for thread safe command insert/delete
		// std::mutex loop_cmd_locker;

		/// keeping track about our pipe is writable or not
		bool pipe_writable = false;

		/// statically allocated memory for reading socket buffer
		char readable_buffer[BASE_LOOP_READABLE_DATA_SIZE];

		/// List of commands which are used to keep them before event loop would have a time to process them
		std::list<loop_cmd_t*> _commands;

		/// just a local data for keeping some reference to our pipe options inside Kernel Loop
		loop_event_data_t pipe_event_data;
		loop_event_data_t tcp_server_event_data;

		static void handle_signal(int sig)
		{
			if (sig == SIGINT)
			{
				std::cerr << BLUE << "SIGINT detected, terminating server now" << std::endl;
				exit(0);
			}
		}

		void	handle_signals(void)
		{
			signal(SIGQUIT, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGINT, handle_signal);
		}

		/// Accepting connections from server socket
		inline void accept_tcp(loop_event_data_t *data)
		{
			if(this->tcp_listener <= 0)
			{
				std::cerr << RED;
				perror("accept_tcp");
				std::cerr << RESET;
				return;
			}

			struct sockaddr addr;
			socklen_t socklen;
			for(;;)
			{
				// handle_signals();
				const int fd = accept(this->tcp_listener, &addr, &socklen);
				if(fd <= 0)
				{
					std::cerr << RED;
					perror("accept");
					std::cerr << RESET;
					break;
				}

				// calling callback function for handling accepted connection
				this->acceptable(data, fd);
			}
		}

		/// Using this function we will resolve given address and then for every resolved address Hint
		/// we will apply "rs" argument function, so that for client it would be 'connect' function for server 'bind'
		inline int resolve_tcp_addr(std::string &address, ResolveFN rs)
		{
			const unsigned long split_index = address.find(':');
			const std::string host = address.substr(0, split_index);
			const std::string port = address.substr(split_index + 1);
			int ret_socket = -1;

			struct addrinfo hints, * res, *rp;
			if (memset(& hints, 0, sizeof hints))
			// Set the attribute for hint
			hints.ai_family = AF_UNSPEC; // We don't care V4 AF_INET or 6 AF_INET6
			hints.ai_socktype = SOCK_STREAM; // TCP Socket SOCK_DGRAM
			hints.ai_flags = AI_PASSIVE;

			// getting address information
			int status = getaddrinfo((host.length() > 0 ? host.c_str() : NULL), port.c_str(), &hints, &res);
			if(status != 0)
			{
				std::cerr << RED;
				perror("getaddrinfo");
				std::cerr << RESET;
				return status;
			}

			for(rp = res; rp != NULL; rp = rp->ai_next)
			{
				handle_signals();
				ret_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				if(ret_socket <= 0)
				{
					std::cerr << RED;
					perror("socket");
					std::cerr << RESET;
					continue;
				}

				this->make_socket_non_blocking(ret_socket);

				status = rs(ret_socket, rp->ai_addr, rp->ai_addrlen);
				if(status >= 0)
				{
					std::cerr << RED;
					perror("make_socket_non_blocking");
					std::cerr << RESET;
					break;
				}

				close(ret_socket);
				ret_socket = -1;
			}

			// cleaning up address result during lookup
			freeaddrinfo(res);

			// checking if we got any address or not
			if(rp == NULL || ret_socket == -1)
			{
				std::cerr << RED;
				perror("196");
				std::cerr << RESET;
				exit(EAI_FAIL);
				return EAI_FAIL;
			}

			return ret_socket;
		}

	protected:
		/// Commands List which are used during execution
		std::list<loop_cmd_t*> commands;

		/// callback for accepting connection here
		virtual void acceptable(loop_event_data_t *data, int fd) = 0;

		/// callback for reading data from socket here
		virtual bool readable(loop_event_data_t *data, char *buffer, size_t read_len) = 0;

		/// callback for writing data to socket
		virtual void writable(loop_event_data_t *data) = 0;

		/// get notification from pipe and read commands
		virtual void notify(loop_cmd_t *cmd){};

		/// callback function for getting connection close event
		virtual void closed(loop_event_data_t *data) = 0;

		/// Binding server on given address
		/// NOTE: address should be in format host:port
		int listen_tcp(std::string &address)
		{
			int status = this->resolve_tcp_addr(address, bind);
			if(status < 0)
			{
				std::cerr << RED;
				perror("resolve_tcp_addr");
				std::cerr << RESET;
				return status;
			}
			this->tcp_listener = status;

			status = listen(this->tcp_listener, UINT16_MAX);
			if(status < 0)
			{
				close(this->tcp_listener);
				this->tcp_listener = -1;
				std::cerr << RED;
				perror("listen");
				std::cerr << RESET;
				return status;
			}

			this->tcp_server_event_data.data = NULL;
			this->tcp_server_event_data.fd = this->tcp_listener;
			this->register_handle(&this->tcp_server_event_data);
			this->make_readable(&this->tcp_server_event_data);

			// if we got here then we have listening tcp socket
			return 0;
		}

		/// Function for making client TCP connections
		/// Unfortunately this function is Blocking function!
		int connect_tcp(std::string &address)
		{
			// resolving address and connecting to it
			return this->resolve_tcp_addr(address, connect);
		}

		/// making connection non blocking for handling async events from kernel Event Loop
		inline int make_socket_non_blocking (int sfd)
		{
			const int flags = fcntl (sfd, F_GETFL, 0);
			if ( flags == -1)
			{
				std::cerr << RED;
				perror("fcntl 272");
				std::cerr << RESET;
				return -1;
			}

			const int temp_flags = flags | O_NONBLOCK;
			const int s = fcntl (sfd, F_SETFL, temp_flags);
			if (s == -1)
			{
				std::cerr << RED;
				perror("fcntl 282");
				std::cerr << RESET;
				return -1;
			}

			return 0;
		}

		/// one time actions which are needed to init base components for event loop
		void init_loop()
		{
			int pipe_chans[2];
			if (pipe(pipe_chans) == -1)
				return(perror("pipe init loop"));
			this->pipe_chan = pipe_chans[1];
			this->make_socket_non_blocking(this->pipe_chan);

#ifdef USE_KQUEUE
			this->event_fd = kqueue();
			if (this->event_fd == -1)
			std::cerr << RED;
				perror("kqueue 303");
				std::cerr << RESET;
#elif defined(USE_EPOLL)
			this->event_fd = epoll_create(1);
			if (this->event_fd == -1)
			std::cerr << RED;
				perror("epoll 309");
				std::cerr << RESET;
#endif
			this->pipe_event_data.data = NULL;
			this->pipe_event_data.fd = this->pipe_chan;
			this->register_handle(&this->pipe_event_data);
		}

		/// Base function for sending commands to this loop
		/// it will make pipe writable and will insert command to our list
		/// so when loop would be ready to consume pipe we will iterate over commands
		inline void send_cmd(loop_cmd_t *cmd)
		{
			// loop_cmd_locker.lock();
			this->_commands.push_back(cmd);

			// not making system call if we did already on one cycle
			// waiting until command will complete, then we would make it writable again
			if(!this->pipe_writable)
			{
				// just making pipe writable to trigger event in the loop
				this->make_writable_one_shot(&this->pipe_event_data, true);
				this->pipe_writable = true; // keeping track of writable pipe
			}

			// loop_cmd_locker.unlock();
		}

		/// Registering Socket handle for this event loop based on environment Epoll or Kqueue
		/// This will make a system call for just adding given handle to Kernel file handles list for consuming events
		/// But at this point we are not registering any events, we will add them as a separate function calls
		inline void register_handle(loop_event_data_t *data)
		{
			if(data == nullptr)
				return;

#ifdef USE_KQUEUE
			struct kevent set_events[4];
			// in any case adding read and write events for this socket, then we will modify it
			// during reregister process
			EV_SET(&set_events[0], data->fd, EVFILT_READ, EV_ADD, 0, 0, (void *)data);
			EV_SET(&set_events[1], data->fd, EVFILT_WRITE, EV_ADD, 0, 0, (void *)data);
			EV_SET(&set_events[2], data->fd, EVFILT_READ, EV_DISABLE, 0, 0, (void *)data);
			EV_SET(&set_events[3], data->fd, EVFILT_WRITE, EV_DISABLE, 0, 0, (void *)data);
			if (kevent(this->event_fd, set_events, 4, NULL, 0, NULL) == -1)
			std::cerr << RED;
				perror("kevent 355");
				std::cerr << RESET;

#elif defined(USE_EPOLL)
			struct epoll_event event;
			event.data.ptr = data;
			if(readable(data, this->readable_buffer, (size_t)BASE_LOOP_READABLE_DATA_SIZE) == true)
				event.events = EPOLLIN;
			else
				event.events = EPOLLIN | EPOLLOUT;
			if (epoll_ctl (this->event_fd, EPOLL_CTL_ADD, data->fd, &event) == -1)
			std::cerr << RED;
				perror("epoll_ctl 367");
				std::cerr << RESET;
#endif
		}

		/// Making socket readable for starting data handling
		/// This will disable "Write" events from this socket
		/// If "one_shot" is true then socket would be registered as "ONE_SHOT" based on OS Epoll or Kqueue
		inline void make_readable(loop_event_data_t *data)
		{
			this->reregister_handle(data, true, false);
		}

		/// Same as "make_readable" but registering event as "One Shot" which means it will trigger only once for this handle
		/// So if we want to get another event we need to reregister this handle again
		inline void make_readable_one_shot(loop_event_data_t *data)
		{
			this->reregister_handle(data, false, true);
		}

		/// Making socket writable for getting event when this socket is ready for writing buffer
		/// This wouldn't disable socket for read events by default, but it is possible to specify "write_only" boolean to true
		inline void make_writable(loop_event_data_t *data, bool write_only = false)
		{
			this->reregister_handle(data, false, false, write_only);
		}

		/// Same as "make_writable" but registering with one shot principle
		inline void make_writable_one_shot(loop_event_data_t *data, bool write_only = false)
		{
			this->reregister_handle(data, false, true, write_only);
		}

		/// Base function for reregistering event handle for this loop
		inline void reregister_handle(loop_event_data_t *data, bool readable, bool one_shot, bool write_only = false)
		{
			if(data == nullptr)
				return;

#ifdef USE_KQUEUE
			struct kevent set_events[2];

			EV_SET(&set_events[1], data->fd, EVFILT_WRITE, (readable ? EV_DISABLE : (one_shot ? EV_ENABLE | EV_ONESHOT : EV_ENABLE)), 0, 0, (void *)data);
			EV_SET(&set_events[0], data->fd, EVFILT_READ, (!readable && write_only ? EV_DISABLE : (one_shot ? EV_ENABLE | EV_ONESHOT : EV_ENABLE)), 0, 0, (void *)data);

			if (kevent(this->event_fd, set_events, 2, NULL, 0, NULL) == -1)
			std::cerr << RED;
				perror("kevent 414");
				std::cerr << RESET;

#elif defined(USE_EPOLL)
			struct epoll_event event;
			event.data.ptr = data;
			if(!writable())
				event.events = EPOLLIN;
			else
				event.events = EPOLLIN | EPOLLOUT;

			if (epoll_ctl (this->event_fd, EPOLL_CTL_MOD, fd, &event) == -1)
			std::cerr << RED;
				perror("epoll_ctl 427");
				std::cerr << RESET;
#endif
		}

		/// delete read, write events from given socket
		void clear_fd(int fd)
		{
#ifdef USE_KQUEUE
			struct kevent set_events[2];
			EV_SET(&set_events[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
			EV_SET(&set_events[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
			if (kevent(this->event_fd, set_events, 2, NULL, 0, NULL) == -1)
			std::cerr << RED;
				perror("kevent 441");
				std::cerr << RESET;
#elif defined(USE_EPOLL)
			struct epoll_event event;
			event.events = EPOLLIN | EPOLLOUT | EPOLLET;
			if (epoll_ctl (this->event_fd, EPOLL_CTL_DEL, fd, &event) == -1)
			std::cerr << RED;
				perror("epoll_ctl 448");
				std::cerr << RESET;
#endif
		}

		/// Closing given handle
		void close_fd(int fd)
		{
			this->clear_fd(fd);
			close(fd);
		}

		/// Base function for reading data and passing it as a callback
		inline void read_data(loop_event_data_t *data)
		{
			ssize_t r;
			for(;;)
			{
				r = read(data->fd, this->readable_buffer, BASE_LOOP_READABLE_DATA_SIZE);
				handle_signals();
				// if we have an error during read process
				if(r <= 0)
				{
					// checking if our socket is still not ready to read data
					if(r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) // checking errno is forbidden, find other way!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						return;

					// closing connection in case of error or EOF
					this->close_fd(data->fd);
					// calling callback function to handle connection close event
					this->closed(data);
					if (r < 0)
					std::cerr << RED;
						perror("read 466");
						std::cerr << RESET;
					break;
				}

				// calling callback for handling data which we got
				this->readable(data, this->readable_buffer, (size_t)r);

				// if we got data less than max buffer size then we have all socket data
				// returning to get back when we will have more data
				if(r < BASE_LOOP_READABLE_DATA_SIZE)
					break;
			}
		}

		/// stopping event loop, but this actually not a thread safe call !!
		void stop_loop()
		{
			close(this->event_fd);
		}

		/// Start base loop running Epoll or Kqueue
		void run_loop()
		{

#ifdef USE_KQUEUE
			struct kevent get_events[BASE_LOOP_EVENTS_COUNT];

			int nev, i;

			while (true)
			{
				nev = kevent(this->event_fd, NULL, 0, get_events, BASE_LOOP_EVENTS_COUNT, NULL);
				if(nev < 0)
				{
					if(errno == EFAULT || errno == ENOMEM || errno == EINTR || errno == EACCES) // checking errno is forbidden!!!!!!!!!!!!!
					{
						std::cerr << RED;
						perror("kevent 518");
						std::cerr << RESET;
						break;
					}
					else
						continue;
				}

				for(i = 0; i < nev; i++)
				{
					handle_signals();
					if(get_events[i].flags & EV_ERROR)
					{
						fprintf(stderr, "Event Loop error -> %s", strerror(get_events[i].data));
						std::cerr << RED;
						perror("532");
						std::cerr << RESET;
						return;
					}
					else if(get_events[i].ident == this->pipe_chan)
					{
						// triggering notification
						// loop_cmd_locker.lock();
						if(!this->_commands.empty())
						{
							// copying commands here for disabling locking here
							this->commands.insert(this->commands.end(), this->_commands.begin(), this->_commands.end());
							// clearing original list for adding to it more
							this->_commands.clear();
						}

						this->pipe_writable = false;
						// loop_cmd_locker.unlock();
						while(!this->commands.empty())
						{
							auto cmd = this->commands.front();
							this->notify(cmd);
							this->commands.pop_front();
							delete cmd;
						}
					}
					else if(get_events[i].ident == this->tcp_listener)
					{
						this->accept_tcp((loop_event_data_t *)get_events[i].udata);
					}
					else if(get_events[i].filter == EVFILT_READ)
					{
						this->read_data((loop_event_data_t *)get_events[i].udata);
					}
					else if(get_events[i].filter == EVFILT_WRITE)
					{
						this->writable((loop_event_data_t *)get_events[i].udata);
					}
				}
			}
#elif defined(USE_EPOLL)

			struct epoll_event get_events[this->max_event_count];

			int nev, i;

			while (true)
			{
				nev = epoll_wait (this->event_fd, get_events, this->max_event_count, -1);
				if(nev < 0)
				{
					if(errno == EBADF || errno == EINVAL)
					{
						std::cerr << RED;
						perror("epoll_wait 580");
						std::cerr << RESET;
						break;
					}
					else
						continue;
				}

				for(i = 0; i < nev; i++)
				{
					// extracting event data
					loop_event_data_t *ev_data = (loop_event_data_t*)get_events[i].data.ptr;

//                    if ((get_events[i].events & EPOLLERR) ||
//                      (get_events[i].events & EPOLLHUP))
//                    {
//                        fprintf (stderr, "epoll error, closing connection !\n");
//                        this->close_fd(ev_data->fd);
//                        continue;
//                    }
//                    else
					if(ev_data->fd == this->pipe_chan)
					{
						// triggering notification
						// loop_cmd_locker.lock();
						if(!this->_commands.empty())
						{
							// copying commands here for disabling locking here
							this->commands.insert(this->commands.end(), this->_commands.begin(), this->_commands.end());
							// clearing original list for adding to it more
							this->_commands.erase(this->_commands.begin());
							this->_commands.clear();
						}
						this->reregister_handle(this->pipe_chan, &this->pipe_event_data, false);
						this->pipe_writable = false;
						// loop_cmd_locker.unlock();
						this->notify(this->commands.front());
					}
					else if(ev_data->fd == this->tcp_listener)
					{
						this->acceptable(ev_data, ev_data->fd);
					}
					else if(get_events[i].events & EPOLLOUT)
					{
						this->writable(ev_data);
					}
					else if((get_events[i].events & EPOLLIN) || (get_events[i].events & EPOLLPRI))
					{
						this->readable(ev_data, this->readable_buffer, BASE_LOOP_READABLE_DATA_SIZE);
					}
				}
			}
#endif
		}
	};
}

#endif //BASELOOP_BASELOOP_H
