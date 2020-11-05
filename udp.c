#include <ws2ipdef.h>

int ff_socket_nonblock(int socket, int enable);

void ff_network_close(void);

int ff_tls_init(void);
void ff_tls_deinit(void);

int ff_network_wait_fd(int fd, int write);


int ff_network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb);


int ff_network_sleep_interruptible(int64_t timeout, AVIOInterruptCB *int_cb);

#if !HAVE_STRUCT_SOCKADDR_STORAGE
struct sockaddr_storage {
#if HAVE_STRUCT_SOCKADDR_SA_LEN
    uint8_t ss_len;
    uint8_t ss_family;
#else
    uint16_t ss_family;
#endif 
    char ss_pad1[6];
    int64_t ss_align;
    char ss_pad2[112];
};
#endif 

typedef union sockaddr_union {
    struct sockaddr_storage storage;
    struct sockaddr_in in;
#if HAVE_STRUCT_SOCKADDR_IN6
    struct sockaddr_in6 in6;
#endif
} sockaddr_union;

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif




#ifndef EAI_AGAIN
#define EAI_AGAIN 2
#endif
#ifndef EAI_BADFLAGS
#define EAI_BADFLAGS 3
#endif
#ifndef EAI_FAIL
#define EAI_FAIL 4
#endif
#ifndef EAI_FAMILY
#define EAI_FAMILY 5
#endif
#ifndef EAI_MEMORY
#define EAI_MEMORY 6
#endif
#ifndef EAI_NODATA
#define EAI_NODATA 7
#endif
#ifndef EAI_NONAME
#define EAI_NONAME 8
#endif
#ifndef EAI_SERVICE
#define EAI_SERVICE 9
#endif
#ifndef EAI_SOCKTYPE
#define EAI_SOCKTYPE 10
#endif

#ifndef AI_PASSIVE
#define AI_PASSIVE 1
#endif

#ifndef AI_CANONNAME
#define AI_CANONNAME 2
#endif

#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 4
#endif

#ifndef NI_NOFQDN
#define NI_NOFQDN 1
#endif

#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 2
#endif

#ifndef NI_NAMERQD
#define NI_NAMERQD 4
#endif

#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV 8
#endif

#ifndef NI_DGRAM
#define NI_DGRAM 16
#endif

#include <stdlib.h>



int ff_getnameinfo(const struct sockaddr *sa, int salen,
                   char *host, int hostlen,
                   char *serv, int servlen, int flags)
{
    const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;

    if (sa->sa_family != AF_INET)
        return EAI_FAMILY;
    if (!host && !serv)
        return EAI_NONAME;

    if (host && hostlen > 0) {
        struct hostent *ent = NULL;
        uint32_t a;
        if (!(flags & NI_NUMERICHOST))
            ent = gethostbyaddr((const char *)&sin->sin_addr,
                                sizeof(sin->sin_addr), AF_INET);

        if (ent) {
            snprintf(host, hostlen, "%s", ent->h_name);
        } else if (flags & NI_NAMERQD) {
            return EAI_NONAME;
        } else {
            a = ntohl(sin->sin_addr.s_addr);
            snprintf(host, hostlen, "%d.%d.%d.%d",
                     ((a >> 24) & 0xff), ((a >> 16) & 0xff),
                     ((a >>  8) & 0xff),  (a        & 0xff));
        }
    }

    if (serv && servlen > 0) {
        if (!(flags & NI_NUMERICSERV))
            return EAI_FAIL;
        snprintf(serv, servlen, "%d", ntohs(sin->sin_port));
    }

    return 0;
}

#if !HAVE_GETADDRINFO || HAVE_WINSOCK2_H
const char *ff_gai_strerror(int ecode)
{
    switch (ecode) {
    case EAI_AGAIN:
        return "Temporary failure in name resolution";
    case EAI_BADFLAGS:
        return "Invalid flags for ai_flags";
    case EAI_FAIL:
        return "A non-recoverable error occurred";
    case EAI_FAMILY:
        return "The address family was not recognized or the address "
               "length was invalid for the specified family";
    case EAI_MEMORY:
        return "Memory allocation failure";
#if EAI_NODATA != EAI_NONAME
    case EAI_NODATA:
        return "No address associated with hostname";
#endif /* EAI_NODATA != EAI_NONAME */
    case EAI_NONAME:
        return "The name does not resolve for the supplied parameters";
    case EAI_SERVICE:
        return "servname not supported for ai_socktype";
    case EAI_SOCKTYPE:
        return "ai_socktype not supported";
    }

    return "Unknown error";
}
#endif /* !HAVE_GETADDRINFO || HAVE_WINSOCK2_H */

int ff_socket_nonblock(int socket, int enable)
{
#if HAVE_WINSOCK2_H
    u_long param = enable;
    return ioctlsocket(socket, FIONBIO, &param);
#else
    if (enable)
        return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
    else
        return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
#endif /* HAVE_WINSOCK2_H */
}

typedef unsigned long nfds_t;

#if !HAVE_POLL_H
int ff_poll(struct pollfd *fds, nfds_t numfds, int timeout)
{
    fd_set read_set;
    fd_set write_set;
    fd_set exception_set;
    nfds_t i;
    int n;
    int rc;

#if HAVE_WINSOCK2_H
    if (numfds >= FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
#endif /* HAVE_WINSOCK2_H */

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&exception_set);

    n = 0;
    for (i = 0; i < numfds; i++) {
        if (fds[i].fd < 0)
            continue;
#if !HAVE_WINSOCK2_H
        if (fds[i].fd >= FD_SETSIZE) {
            errno = EINVAL;
            return -1;
        }
#endif /* !HAVE_WINSOCK2_H */

        if (fds[i].events & POLLIN)
            FD_SET(fds[i].fd, &read_set);
        if (fds[i].events & POLLOUT)
            FD_SET(fds[i].fd, &write_set);
        if (fds[i].events & POLLERR)
            FD_SET(fds[i].fd, &exception_set);

        if (fds[i].fd >= n)
            n = fds[i].fd + 1;
    }

    if (n == 0)
        /* Hey!? Nothing to ff_poll, in fact!!! */
        return 0;

    if (timeout < 0) {
        rc = select(n, &read_set, &write_set, &exception_set, NULL);
    } else {
        struct timeval tv;
        tv.tv_sec  = timeout / 1000;
        tv.tv_usec = 1000 * (timeout % 1000);
        rc         = select(n, &read_set, &write_set, &exception_set, &tv);
    }

    if (rc < 0)
        return rc;

    for (i = 0; i < numfds; i++) {
        fds[i].revents = 0;

        if (FD_ISSET(fds[i].fd, &read_set))
            fds[i].revents |= POLLIN;
        if (FD_ISSET(fds[i].fd, &write_set))
            fds[i].revents |= POLLOUT;
        if (FD_ISSET(fds[i].fd, &exception_set))
            fds[i].revents |= POLLERR;
    }

    return rc;
}
#endif /* !HAVE_POLL_H */

int ff_is_multicast_address(struct sockaddr *addr);

#define POLLING_TIME 100 /// Time in milliseconds between interrupt check

int ff_lock_avformat(void)
{
    return ff_mutex_lock(&avformat_mutex) ? -1 : 0;
}

int ff_unlock_avformat(void)
{
    return ff_mutex_unlock(&avformat_mutex) ? -1 : 0;
}

int ff_tls_init(void)
{
#if CONFIG_TLS_PROTOCOL
#if CONFIG_OPENSSL
    int ret;
    if ((ret = ff_openssl_init()) < 0)
        return ret;
#endif

#endif
    return 0;
}

void ff_tls_deinit(void)
{
#if CONFIG_TLS_PROTOCOL
#if CONFIG_OPENSSL
    ff_openssl_deinit();
#endif
#endif
}



#if HAVE_WINSOCK2_H
int ff_neterrno(void)
{
    int err = WSAGetLastError();
    switch (err) {
    case WSAEWOULDBLOCK:
        return AVERROR(EAGAIN);
    case WSAEINTR:
        return AVERROR(EINTR);
    case WSAEPROTONOSUPPORT:
        return AVERROR(EPROTONOSUPPORT);
    case WSAETIMEDOUT:
        return AVERROR(ETIMEDOUT);
    case WSAECONNREFUSED:
        return AVERROR(ECONNREFUSED);
    case WSAEINPROGRESS:
        return AVERROR(EINPROGRESS);
    }
    return -err;
}
#endif

int ff_network_wait_fd(int fd, int write)
{
    int ev = write ? POLLOUT : POLLIN;
    struct pollfd p = { .fd = fd, .events = ev, .revents = 0 };
    int ret;
    ret = ff_poll(&p, 1, POLLING_TIME);
    return ret < 0 ? ff_neterrno() : p.revents & (ev | POLLERR | POLLHUP) ? 0 : AVERROR(EAGAIN);
}

int ff_network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb)
{
    int ret;
    int64_t wait_start = 0;

    while (1) {
        if (ff_check_interrupt(int_cb))
            return AVERROR_EXIT;
        ret = ff_network_wait_fd(fd, write);
        if (ret != AVERROR(EAGAIN))
            return ret;
        if (timeout > 0) {
            if (!wait_start)
                wait_start = av_gettime_relative();
            else if (av_gettime_relative() - wait_start > timeout)
                return AVERROR(ETIMEDOUT);
        }
    }
}

int ff_network_sleep_interruptible(int64_t timeout, AVIOInterruptCB *int_cb)
{
    int64_t wait_start = av_gettime_relative();

    while (1) {
        int64_t time_left;

        if (ff_check_interrupt(int_cb))
            return AVERROR_EXIT;

        time_left = timeout - (av_gettime_relative() - wait_start);
        if (time_left <= 0)
            return AVERROR(ETIMEDOUT);

        av_usleep(FFMIN(time_left, POLLING_TIME * 1000));
    }
}


int ff_is_multicast_address(struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        return IN_MULTICAST(ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr));
    }
#if HAVE_STRUCT_SOCKADDR_IN6
    if (addr->sa_family == AF_INET6) {
        return IN6_IS_ADDR_MULTICAST(&((struct sockaddr_in6 *)addr)->sin6_addr);
    }
#endif

    return 0;
}

static int ff_poll_interrupt(struct pollfd *p, nfds_t nfds, int timeout, AVIOInterruptCB *cb)
{
    int runs = timeout / POLLING_TIME;
    int ret = 0;

    do {
        if (ff_check_interrupt(cb))
            return AVERROR_EXIT;
        ret = ff_poll(p, nfds, POLLING_TIME);
        if (ret != 0) {
            if (ret < 0)
                ret = ff_neterrno();
            if (ret == AVERROR(EINTR))
                continue;
            break;
        }
    } while (timeout <= 0 || runs-- > 0);

    if (!ret)
        return AVERROR(ETIMEDOUT);
    return ret;
}

int ff_socket(int af, int type, int proto)
{
    int fd;

#ifdef SOCK_CLOEXEC
    fd = socket(af, type | SOCK_CLOEXEC, proto);
    if (fd == -1 && errno == EINVAL)
#endif
    {
        fd = socket(af, type, proto);
#if HAVE_FCNTL
        if (fd != -1) {
            if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
                av_log(NULL, AV_LOG_DEBUG, "Failed to set close on exec\n");
        }
#endif
    }
#ifdef SO_NOSIGPIPE
    if (fd != -1) {
        if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){1}, sizeof(int))) {
             av_log(NULL, AV_LOG_WARNING, "setsockopt(SO_NOSIGPIPE) failed\n");
        }
    }
#endif
    return fd;
}

int ff_listen(int fd, const struct sockaddr *addr,int addrlen)
{
    int ret;
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse, sizeof(reuse))) {
        av_log(NULL, AV_LOG_WARNING, "setsockopt(SO_REUSEADDR) failed\n");
    }
    ret = bind(fd, addr, addrlen);
    if (ret)
        return ff_neterrno();

    ret = listen(fd, 1);
    if (ret)
        return ff_neterrno();
    return ret;
}

int ff_accept(int fd, int timeout, URLContext *h)
{
    int ret;
    struct pollfd lp = { fd, POLLIN, 0 };

    ret = ff_poll_interrupt(&lp, 1, timeout, &h->interrupt_callback);
    if (ret < 0)
        return ret;

    ret = accept(fd, NULL, NULL);
    if (ret < 0)
        return ff_neterrno();
    if (ff_socket_nonblock(ret, 1) < 0)
        av_log(h, AV_LOG_DEBUG, "ff_socket_nonblock failed\n");

    return ret;
}

int ff_listen_bind(int fd, const struct sockaddr *addr, int addrlen, int timeout, URLContext *h)
{
    int ret;
    if ((ret = ff_listen(fd, addr, addrlen)) < 0)
        return ret;
    if ((ret = ff_accept(fd, timeout, h)) < 0)
        return ret;
    closesocket(fd);
    return ret;
}

int ff_listen_connect(int fd, const struct sockaddr *addr,int addrlen, int timeout, URLContext *h,int will_try_next)
{
    struct pollfd p = {fd, POLLOUT, 0};
    int ret;
    int optlen;

    if (ff_socket_nonblock(fd, 1) < 0)
        av_log(h, AV_LOG_DEBUG, "ff_socket_nonblock failed\n");

    while ((ret = connect(fd, addr, addrlen))) {
        ret = ff_neterrno();
        switch (ret) {
        case AVERROR(EINTR):
            if (ff_check_interrupt(&h->interrupt_callback))
                return AVERROR_EXIT;
            continue;
        case AVERROR(EINPROGRESS):
        case AVERROR(EAGAIN):
            ret = ff_poll_interrupt(&p, 1, timeout, &h->interrupt_callback);
            if (ret < 0)
                return ret;
            optlen = sizeof(ret);
            if (getsockopt (fd, SOL_SOCKET, SO_ERROR, (void*)&ret, &optlen))
                ret = AVUNERROR(ff_neterrno());
            if (ret != 0) {
                char errbuf[100];
                ret = AVERROR(ret);
                av_strerror(ret, errbuf, sizeof(errbuf));
                if (will_try_next)
                    av_log(h, AV_LOG_WARNING,"Connection to %s failed (%s), trying next address\n",h->filename, errbuf);
                else
                    av_log(h, AV_LOG_ERROR, "Connection to %s failed: %s\n", h->filename, errbuf);
            }
        default:
            return ret;
        }
    }
    return ret;
}

static void interleave_addrinfo(struct addrinfo *base)
{
    struct addrinfo **next = &base->ai_next;
    while (*next) {
        struct addrinfo *cur = *next;
        // Iterate forward until we find an entry of a different family.
        if (cur->ai_family == base->ai_family) {
            next = &cur->ai_next;
            continue;
        }
        if (cur == base->ai_next) {
            // If the first one following base is of a different family, just
            // move base forward one step and continue.
            base = cur;
            next = &base->ai_next;
            continue;
        }
        // Unchain cur from the rest of the list from its current spot.
        *next = cur->ai_next;
        // Hook in cur directly after base.
        cur->ai_next = base->ai_next;
        base->ai_next = cur;
        // Restart with a new base. We know that before moving the cur element,
        // everything between the previous base and cur had the same family,
        // different from cur->ai_family. Therefore, we can keep next pointing
        // where it was, and continue from there with base at the one after
        // cur.
        base = cur->ai_next;
    }
}

static void print_address_list(void *ctx, const struct addrinfo *addr,
                               const char *title)
{
    char hostbuf[100], portbuf[20];
    av_log(ctx, AV_LOG_DEBUG, "%s:\n", title);
    while (addr) {
        getnameinfo(addr->ai_addr, addr->ai_addrlen,
                    hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        av_log(ctx, AV_LOG_DEBUG, "Address %s port %s\n", hostbuf, portbuf);
        addr = addr->ai_next;
    }
}

struct ConnectionAttempt {
    int fd;
    int64_t deadline_us;
    struct addrinfo *addr;
};

// Returns < 0 on error, 0 on successfully started connection attempt,
// > 0 for a connection that succeeded already.
static int start_connect_attempt(struct ConnectionAttempt *attempt,
                                 struct addrinfo **ptr, int timeout_ms,
                                 URLContext *h,
                                 void (*customize_fd)(void *, int), void *customize_ctx)
{
    struct addrinfo *ai = *ptr;
    int ret;

    *ptr = ai->ai_next;

    attempt->fd = ff_socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (attempt->fd < 0)
        return ff_neterrno();
    attempt->deadline_us = av_gettime_relative() + timeout_ms * 1000;
    attempt->addr = ai;

    ff_socket_nonblock(attempt->fd, 1);

    if (customize_fd)
        customize_fd(customize_ctx, attempt->fd);

    while ((ret = connect(attempt->fd, ai->ai_addr, ai->ai_addrlen))) {
        ret = ff_neterrno();
        switch (ret) {
        case AVERROR(EINTR):
            if (ff_check_interrupt(&h->interrupt_callback)) {
                closesocket(attempt->fd);
                attempt->fd = -1;
                return AVERROR_EXIT;
            }
            continue;
        case AVERROR(EINPROGRESS):
        case AVERROR(EAGAIN):
            return 0;
        default:
            closesocket(attempt->fd);
            attempt->fd = -1;
            return ret;
        }
    }
    return 1;
}

// Try a new connection to another address after 200 ms, as suggested in
// RFC 8305 (or sooner if an earlier attempt fails).
#define NEXT_ATTEMPT_DELAY_MS 200

int ff_connect_parallel(struct addrinfo *addrs, int timeout_ms_per_address,
                        int parallel, URLContext *h, int *fd,
                        void (*customize_fd)(void *, int), void *customize_ctx)
{
    struct ConnectionAttempt attempts[3];
    struct pollfd pfd[3];
    int nb_attempts = 0, i, j;
    int64_t next_attempt_us = av_gettime_relative(), next_deadline_us;
    int last_err = AVERROR(EIO);
    int optlen;
    char errbuf[100], hostbuf[100], portbuf[20];

    if (parallel > FF_ARRAY_ELEMS(attempts))
        parallel = FF_ARRAY_ELEMS(attempts);

    print_address_list(h, addrs, "Original list of addresses");
    // This mutates the list, but the head of the list is still the same
    // element, so the caller, who owns the list, doesn't need to get
    // an updated pointer.
    interleave_addrinfo(addrs);
    print_address_list(h, addrs, "Interleaved list of addresses");

    while (nb_attempts > 0 || addrs) {
        // Start a new connection attempt, if possible.
        if (nb_attempts < parallel && addrs) {
            getnameinfo(addrs->ai_addr, addrs->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            av_log(h, AV_LOG_VERBOSE, "Starting connection attempt to %s port %s\n",
                                      hostbuf, portbuf);
            last_err = start_connect_attempt(&attempts[nb_attempts], &addrs,
                                             timeout_ms_per_address, h,
                                             customize_fd, customize_ctx);
            if (last_err < 0) {
                av_strerror(last_err, errbuf, sizeof(errbuf));
                av_log(h, AV_LOG_VERBOSE, "Connected attempt failed: %s\n",
                                          errbuf);
                continue;
            }
            if (last_err > 0) {
                for (i = 0; i < nb_attempts; i++)
                    closesocket(attempts[i].fd);
                *fd = attempts[nb_attempts].fd;
                return 0;
            }
            pfd[nb_attempts].fd = attempts[nb_attempts].fd;
            pfd[nb_attempts].events = POLLOUT;
            next_attempt_us = av_gettime_relative() + NEXT_ATTEMPT_DELAY_MS * 1000;
            nb_attempts++;
        }

        av_assert0(nb_attempts > 0);
        // The connection attempts are sorted from oldest to newest, so the
        // first one will have the earliest deadline.
        next_deadline_us = attempts[0].deadline_us;
        // If we can start another attempt in parallel, wait until that time.
        if (nb_attempts < parallel && addrs)
            next_deadline_us = FFMIN(next_deadline_us, next_attempt_us);
        last_err = ff_poll_interrupt(pfd, nb_attempts,
                                     (next_deadline_us - av_gettime_relative())/1000,
                                     &h->interrupt_callback);
        if (last_err < 0 && last_err != AVERROR(ETIMEDOUT))
            break;

        // Check the status from the ff_poll output.
        for (i = 0; i < nb_attempts; i++) {
            last_err = 0;
            if (pfd[i].revents) {
                // Some sort of action for this socket, check its status (either
                // a successful connection or an error).
                optlen = sizeof(last_err);
                if (getsockopt(attempts[i].fd, SOL_SOCKET, SO_ERROR, (void*)&last_err, &optlen))
                    last_err = ff_neterrno();
                else if (last_err != 0)
                    last_err = AVERROR(last_err);
                if (last_err == 0) {
                    // Everything is ok, we seem to have a successful
                    // connection. Close other sockets and return this one.
                    for (j = 0; j < nb_attempts; j++)
                        if (j != i)
                            closesocket(attempts[j].fd);
                    *fd = attempts[i].fd;
                    getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                                hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    av_log(h, AV_LOG_VERBOSE, "Successfully connected to %s port %s\n",
                                              hostbuf, portbuf);
                    return 0;
                }
            }
            if (attempts[i].deadline_us < av_gettime_relative() && !last_err)
                last_err = AVERROR(ETIMEDOUT);
            if (!last_err)
                continue;
            // Error (or timeout) for this socket; close the socket and remove
            // it from the attempts/pfd arrays, to let a new attempt start
            // directly.
            getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            av_strerror(last_err, errbuf, sizeof(errbuf));
            av_log(h, AV_LOG_VERBOSE, "Connection attempt to %s port %s "
                                      "failed: %s\n", hostbuf, portbuf, errbuf);
            closesocket(attempts[i].fd);
            memmove(&attempts[i], &attempts[i + 1],
                    (nb_attempts - i - 1) * sizeof(*attempts));
            memmove(&pfd[i], &pfd[i + 1],
                    (nb_attempts - i - 1) * sizeof(*pfd));
            i--;
            nb_attempts--;
        }
    }
    for (i = 0; i < nb_attempts; i++)
        closesocket(attempts[i].fd);
    if (last_err >= 0)
        last_err = AVERROR(ECONNREFUSED);
    if (last_err != AVERROR_EXIT) {
        av_strerror(last_err, errbuf, sizeof(errbuf));
        av_log(h, AV_LOG_ERROR, "Connection to %s failed: %s\n",
               h->filename, errbuf);
    }
    return last_err;
}



void ff_log_net_error(void *ctx, int level, const char* prefix)
{
    char errbuf[100];
    av_strerror(ff_neterrno(), errbuf, sizeof(errbuf));
    av_log(ctx, level, "%s: %s\n", prefix, errbuf);
}
#define OFFSET(x) offsetof(UDPContext, x)

static const AVOption options[] = {
    { "buffer_size",    "System data size (in bytes)",                     OFFSET(buffer_size),    AV_OPT_TYPE_INT,    { .i64 = -1 },    -1, INT_MAX, .flags = 3 },
    { "bitrate",        "Bits to send per second",                         OFFSET(bitrate),        AV_OPT_TYPE_INT64,  { .i64 = 0  },     0, INT64_MAX, .flags = 1 },
    { "burst_bits",     "Max length of bursts in bits (when using bitrate)", OFFSET(burst_bits),   AV_OPT_TYPE_INT64,  { .i64 = 0  },     0, INT64_MAX, .flags = 1 },
    { "localport",      "Local port",                                      OFFSET(local_port),     AV_OPT_TYPE_INT,    { .i64 = -1 },    -1, INT_MAX, 3 },
    { "local_port",     "Local port",                                      OFFSET(local_port),     AV_OPT_TYPE_INT,    { .i64 = -1 },    -1, INT_MAX, .flags = 3 },
    { "localaddr",      "Local address",                                   OFFSET(localaddr),      AV_OPT_TYPE_STRING, { .str = NULL },               .flags = 3 },
    { "udplite_coverage", "choose UDPLite head size which should be validated by checksum", OFFSET(udplite_coverage), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, 3 },
    { "pkt_size",       "Maximum UDP packet size",                         OFFSET(pkt_size),       AV_OPT_TYPE_INT,    { .i64 = 1472 },  -1, INT_MAX, .flags = 3 },
    { "reuse",          "explicitly allow reusing UDP sockets",            OFFSET(reuse_socket),   AV_OPT_TYPE_BOOL,   { .i64 = -1 },    -1, 1,       3 },
    { "reuse_socket",   "explicitly allow reusing UDP sockets",            OFFSET(reuse_socket),   AV_OPT_TYPE_BOOL,   { .i64 = -1 },    -1, 1,       .flags = 3 },
    { "broadcast", "explicitly allow or disallow broadcast destination",   OFFSET(is_broadcast),   AV_OPT_TYPE_BOOL,   { .i64 = 0  },     0, 1,       1 },
    { "ttl",            "Time to live (multicast only)",                   OFFSET(ttl),            AV_OPT_TYPE_INT,    { .i64 = 16 },     0, INT_MAX, 1 },
    { "connect",        "set if connect() should be called on socket",     OFFSET(is_connected),   AV_OPT_TYPE_BOOL,   { .i64 =  0 },     0, 1,       .flags = 3 },
    { "fifo_size",      "set the UDP receiving circular buffer size, expressed as a number of packets with size of 188 bytes", OFFSET(circular_buffer_size), AV_OPT_TYPE_INT, {.i64 = 7*4096}, 0, INT_MAX, 2 },
    { "overrun_nonfatal", "survive in case of UDP receiving circular buffer overrun", OFFSET(overrun_nonfatal), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1,    2 },
    { "timeout",        "set raise error timeout, in microseconds (only in read mode)",OFFSET(timeout),         AV_OPT_TYPE_INT,  {.i64 = 0}, 0, INT_MAX, 2 },
    { "sources",        "Source list",                                     OFFSET(sources),        AV_OPT_TYPE_STRING, { .str = NULL },               .flags = 3 },
    { "block",          "Block list",                                      OFFSET(block),          AV_OPT_TYPE_STRING, { .str = NULL },               .flags = 3 },
    { NULL }
};

static const AVClass udp_class = {
    .class_name = "udp",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static const AVClass udplite_context_class = {
    .class_name     = "udplite",
    .item_name      = av_default_item_name,
    .option         = options,
    .version        = LIBAVUTIL_VERSION_INT,
};








static int compare_addr(const struct sockaddr_storage *a,
                        const struct sockaddr_storage *b)
{
    if (a->ss_family != b->ss_family)
        return 1;
    if (a->ss_family == AF_INET) {
        return (((const struct sockaddr_in *)a)->sin_addr.s_addr !=
                ((const struct sockaddr_in *)b)->sin_addr.s_addr);
    }

#if HAVE_STRUCT_SOCKADDR_IN6
    if (a->ss_family == AF_INET6) {
        const uint8_t *s6_addr_a = ((const struct sockaddr_in6 *)a)->sin6_addr.s6_addr;
        const uint8_t *s6_addr_b = ((const struct sockaddr_in6 *)b)->sin6_addr.s6_addr;
        return memcmp(s6_addr_a, s6_addr_b, 16);
    }
#endif
    return 1;
}

int ff_ip_check_source_lists(struct sockaddr_storage *source_addr_ptr, IPSourceFilters *s)
{
    int i;
    if (s->nb_exclude_addrs) {
        for (i = 0; i < s->nb_exclude_addrs; i++) {
            if (!compare_addr(source_addr_ptr, &s->exclude_addrs[i]))
                return 1;
        }
    }
    if (s->nb_include_addrs) {
        for (i = 0; i < s->nb_include_addrs; i++) {
            if (!compare_addr(source_addr_ptr, &s->include_addrs[i]))
                return 0;
        }
        return 1;
    }
    return 0;
}



struct addrinfo *ff_ip_resolve_host(void *log_ctx,
                                    const char *hostname, int port,
                                    int type, int family, int flags)
{
    struct addrinfo hints = { 0 }, *res = 0;
    int error;
    char sport[16];
    const char *node = 0, *service = "0";

    if (port > 0) {
        snprintf(sport, sizeof(sport), "%d", port);
        service = sport;
    }
    if ((hostname) && (hostname[0] != '\0') && (hostname[0] != '?')) {
        node = hostname;
    }
    hints.ai_socktype = type;
    hints.ai_family   = family;
    hints.ai_flags    = flags;
    if ((error = getaddrinfo(node, service, &hints, &res))) {
        res = NULL;
        av_log(log_ctx, AV_LOG_ERROR, "getaddrinfo(%s, %s): %s\n",
               node ? node : "unknown",
               service,
               gai_strerror(error));
    }

    return res;
}


static int ip_parse_addr_list(void *log_ctx, const char *buf,
                              struct sockaddr_storage **address_list_ptr,
                              int *address_list_size_ptr)
{
    struct addrinfo *ai = NULL;

    

    while (buf && buf[0]) {
        char* host = av_get_token(&buf, ",");
        if (!host)
            return AVERROR(ENOMEM);

        ai = ff_ip_resolve_host(log_ctx, host, 0, SOCK_DGRAM, AF_UNSPEC, 0);
        av_freep(&host);

        if (ai) {
            struct sockaddr_storage source_addr = {0};
            memcpy(&source_addr, ai->ai_addr, ai->ai_addrlen);
            freeaddrinfo(ai);
            av_dynarray2_add((void **)address_list_ptr, address_list_size_ptr, sizeof(source_addr), (uint8_t *)&source_addr);
            if (!*address_list_ptr)
                return AVERROR(ENOMEM);
        } else {
            return AVERROR(EINVAL);
        }

        if (*buf)
            buf++;
    }

    return 0;
}

static int ip_parse_sources_and_blocks(void *log_ctx, const char *buf, IPSourceFilters *filters, int parse_include_list)
{
    int ret;
    if (parse_include_list)
        ret = ip_parse_addr_list(log_ctx, buf, &filters->include_addrs, &filters->nb_include_addrs);
    else
        ret = ip_parse_addr_list(log_ctx, buf, &filters->exclude_addrs, &filters->nb_exclude_addrs);

    if (ret >= 0 && filters->nb_include_addrs && filters->nb_exclude_addrs) {
        av_log(log_ctx, AV_LOG_ERROR, "Simultaneously including and excluding sources is not supported.\n");
        return AVERROR(EINVAL);
    }
    return ret;
}

int ff_ip_parse_sources(void *log_ctx, const char *buf, IPSourceFilters *filters)
{
    return ip_parse_sources_and_blocks(log_ctx, buf, filters, 1);
}

int ff_ip_parse_blocks(void *log_ctx, const char *buf, IPSourceFilters *filters)
{
    return ip_parse_sources_and_blocks(log_ctx, buf, filters, 0);
}

void ff_ip_reset_filters(IPSourceFilters *filters)
{
    av_freep(&filters->exclude_addrs);
    av_freep(&filters->include_addrs);
    filters->nb_include_addrs = 0;
    filters->nb_exclude_addrs = 0;
}



static int udp_set_multicast_ttl(int sockfd, int mcastTTL,struct sockaddr *addr)
{
#ifdef IP_MULTICAST_TTL
    if (addr->sa_family == AF_INET) {
        if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (const void*)&mcastTTL, sizeof(mcastTTL)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IP_MULTICAST_TTL)");
            return -1;
        }
    }
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_MULTICAST_HOPS)
    if (addr->sa_family == AF_INET6) {
        if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const void *)&mcastTTL, sizeof(mcastTTL)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IPV6_MULTICAST_HOPS)");
            return -1;
        }
    }
#endif
    return 0;
}

static int udp_join_multicast_group(int sockfd, struct sockaddr *addr,struct sockaddr *local_addr)
{
#ifdef IP_ADD_MEMBERSHIP
    if (addr->sa_family == AF_INET) {
        struct ip_mreq mreq;

        mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
        if (local_addr)
            mreq.imr_interface= ((struct sockaddr_in *)local_addr)->sin_addr;
        else
            mreq.imr_interface.s_addr = INADDR_ANY;
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void *)&mreq, sizeof(mreq)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IP_ADD_MEMBERSHIP)");
            return -1;
        }
    }
#endif
#if HAVE_STRUCT_IPV6_MREQ && defined(IPPROTO_IPV6)
    if (addr->sa_family == AF_INET6) {
        struct ipv6_mreq mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr, &(((struct sockaddr_in6 *)addr)->sin6_addr), sizeof(struct in6_addr));
        //TODO: Interface index should be looked up from local_addr
        mreq6.ipv6mr_interface = 0;
        if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (const void *)&mreq6, sizeof(mreq6)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IPV6_ADD_MEMBERSHIP)");
            return -1;
        }
    }
#endif
    return 0;
}

static int udp_leave_multicast_group(int sockfd, struct sockaddr *addr,struct sockaddr *local_addr)
{
#ifdef IP_DROP_MEMBERSHIP
    if (addr->sa_family == AF_INET) {
        struct ip_mreq mreq;

        mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
        if (local_addr)
            mreq.imr_interface = ((struct sockaddr_in *)local_addr)->sin_addr;
        else
            mreq.imr_interface.s_addr = INADDR_ANY;
        if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void *)&mreq, sizeof(mreq)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IP_DROP_MEMBERSHIP)");
            return -1;
        }
    }
#endif
#if HAVE_STRUCT_IPV6_MREQ && defined(IPPROTO_IPV6)
    if (addr->sa_family == AF_INET6) {
        struct ipv6_mreq mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr, &(((struct sockaddr_in6 *)addr)->sin6_addr), sizeof(struct in6_addr));
        //TODO: Interface index should be looked up from local_addr
        mreq6.ipv6mr_interface = 0;
        if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (const void *)&mreq6, sizeof(mreq6)) < 0) {
            ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(IPV6_DROP_MEMBERSHIP)");
            return -1;
        }
    }
#endif
    return 0;
}

static int udp_set_multicast_sources(URLContext *h,
                                     int sockfd, struct sockaddr *addr,
                                     int addr_len, struct sockaddr_storage *local_addr,
                                     struct sockaddr_storage *sources,
                                     int nb_sources, int include)
{
    int i;
    if (addr->sa_family != AF_INET) {
#if HAVE_STRUCT_GROUP_SOURCE_REQ && defined(MCAST_BLOCK_SOURCE)
        
        int i;
        for (i = 0; i < nb_sources; i++) {
            struct group_source_req mreqs;
            int level = addr->sa_family == AF_INET ? IPPROTO_IP : IPPROTO_IPV6;

            //TODO: Interface index should be looked up from local_addr
            mreqs.gsr_interface = 0;
            memcpy(&mreqs.gsr_group, addr, addr_len);
            memcpy(&mreqs.gsr_source, &sources[i], sizeof(*sources));

            if (setsockopt(sockfd, level,
                           include ? MCAST_JOIN_SOURCE_GROUP : MCAST_BLOCK_SOURCE,
                           (const void *)&mreqs, sizeof(mreqs)) < 0) {
                if (include)
                    ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(MCAST_JOIN_SOURCE_GROUP)");
                else
                    ff_log_net_error(NULL, AV_LOG_ERROR, "setsockopt(MCAST_BLOCK_SOURCE)");
                return ff_neterrno();
            }
        }
        return 0;
#else
        av_log(h, AV_LOG_ERROR,
               "Setting multicast sources only supported for IPv4\n");
        return AVERROR(EINVAL);
#endif
    }
#if HAVE_STRUCT_IP_MREQ_SOURCE && defined(IP_BLOCK_SOURCE)
    for (i = 0; i < nb_sources; i++) {
        struct ip_mreq_source mreqs;
        if (sources[i].ss_family != AF_INET) {
            av_log(h, AV_LOG_ERROR, "Source/block address %d is of incorrect protocol family\n", i + 1);
            return AVERROR(EINVAL);
        }

        mreqs.imr_multiaddr.s_addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
        if (local_addr)
            mreqs.imr_interface = ((struct sockaddr_in *)local_addr)->sin_addr;
        else
            mreqs.imr_interface.s_addr = INADDR_ANY;
        mreqs.imr_sourceaddr.s_addr = ((struct sockaddr_in *)&sources[i])->sin_addr.s_addr;

        if (setsockopt(sockfd, IPPROTO_IP,
                       include ? IP_ADD_SOURCE_MEMBERSHIP : IP_BLOCK_SOURCE,
                       (const void *)&mreqs, sizeof(mreqs)) < 0) {
            if (include)
                ff_log_net_error(h, AV_LOG_ERROR, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP)");
            else
                ff_log_net_error(h, AV_LOG_ERROR, "setsockopt(IP_BLOCK_SOURCE)");
            return ff_neterrno();
        }
    }
#else
    return AVERROR(ENOSYS);
#endif
    return 0;
}
static int udp_set_url(URLContext *h,
                       struct sockaddr_storage *addr,
                       const char *hostname, int port)
{
    struct addrinfo *res0;
    int addr_len;

    res0 = ff_ip_resolve_host(h, hostname, port, SOCK_DGRAM, AF_UNSPEC, 0);
    if (!res0) return AVERROR(EIO);
    memcpy(addr, res0->ai_addr, res0->ai_addrlen);
    addr_len = res0->ai_addrlen;
    freeaddrinfo(res0);

    return addr_len;
}

static int udp_socket_create(URLContext *h, struct sockaddr_storage *addr,
                             int *addr_len, const char *localaddr)
{
    UDPContext *s = h->priv_data;
    int udp_fd = -1;
    struct addrinfo *res0, *res;
    int family = AF_UNSPEC;

    if (((struct sockaddr *) &s->dest_addr)->sa_family)
        family = ((struct sockaddr *) &s->dest_addr)->sa_family;
    res0 = ff_ip_resolve_host(h, (localaddr && localaddr[0]) ? localaddr : NULL,
                            s->local_port,
                            SOCK_DGRAM, family, AI_PASSIVE);
    if (!res0)
        goto fail;
    for (res = res0; res; res=res->ai_next) {
        if (s->udplite_coverage)
            udp_fd = ff_socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDPLITE);
        else
            udp_fd = ff_socket(res->ai_family, SOCK_DGRAM, 0);
        if (udp_fd != -1) break;
        ff_log_net_error(NULL, AV_LOG_ERROR, "socket");
    }

    if (udp_fd < 0)
        goto fail;

    memcpy(addr, res->ai_addr, res->ai_addrlen);
    *addr_len = res->ai_addrlen;

    freeaddrinfo(res0);

    return udp_fd;

 fail:
    if (udp_fd >= 0)
        closesocket(udp_fd);
    if(res0)
        freeaddrinfo(res0);
    return -1;
}

static int udp_port(struct sockaddr_storage *addr, int addr_len)
{
    char sbuf[sizeof(int)*3+1];
    int error;

    if ((error = getnameinfo((struct sockaddr *)addr, addr_len, NULL, 0,  sbuf, sizeof(sbuf), NI_NUMERICSERV)) != 0) {
        av_log(NULL, AV_LOG_ERROR, "getnameinfo: %s\n", gai_strerror(error));
        return -1;
    }

    return strtol(sbuf, NULL, 10);
}



int ff_udp_set_remote_url(URLContext *h, const char *uri)
{
    UDPContext *s = h->priv_data;
    char hostname[256], buf[10];
    int port;
    const char *p;

    av_url_split(NULL, 0, NULL, 0, hostname, sizeof(hostname), &port, NULL, 0, uri);

    
    s->dest_addr_len = udp_set_url(h, &s->dest_addr, hostname, port);
    if (s->dest_addr_len < 0) {
        return AVERROR(EIO);
    }
    s->is_multicast = ff_is_multicast_address((struct sockaddr*) &s->dest_addr);
    p = strchr(uri, '?');
    if (p) {
        if (av_find_info_tag(buf, sizeof(buf), "connect", p)) {
            int was_connected = s->is_connected;
            s->is_connected = strtol(buf, NULL, 10);
            if (s->is_connected && !was_connected) {
                if (connect(s->udp_fd, (struct sockaddr *) &s->dest_addr,
                            s->dest_addr_len)) {
                    s->is_connected = 0;
                    ff_log_net_error(h, AV_LOG_ERROR, "connect");
                    return AVERROR(EIO);
                }
            }
        }
    }

    return 0;
}


int ff_udp_get_local_port(URLContext *h)
{
    UDPContext *s = h->priv_data;
    return s->local_port;
}


static int udp_get_file_handle(URLContext *h)
{
    UDPContext *s = h->priv_data;
    return s->udp_fd;
}

#if HAVE_PTHREAD_CANCEL
static void *circular_buffer_task_rx( void *_URLContext)
{
    URLContext *h = _URLContext;
    UDPContext *s = h->priv_data;
    int old_cancelstate;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate);
    pthread_mutex_lock(&s->mutex);
    if (ff_socket_nonblock(s->udp_fd, 0) < 0) {
        av_log(h, AV_LOG_ERROR, "Failed to set blocking mode");
        s->circular_buffer_error = AVERROR(EIO);
        goto end;
    }
    while(1) {
        int len;
        struct sockaddr_storage addr;
        int addr_len = sizeof(addr);

        pthread_mutex_unlock(&s->mutex);
        
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancelstate);
        len = recvfrom(s->udp_fd, s->tmp+4, sizeof(s->tmp)-4, 0, (struct sockaddr *)&addr, &addr_len);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate);
        pthread_mutex_lock(&s->mutex);
        if (len < 0) {
            if (ff_neterrno() != AVERROR(EAGAIN) && ff_neterrno() != AVERROR(EINTR)) {
                s->circular_buffer_error = ff_neterrno();
                goto end;
            }
            continue;
        }
        if (ff_ip_check_source_lists(&addr, &s->filters))
            continue;
        AV_WL32(s->tmp, len);

        if(av_fifo_space(s->fifo) < len + 4) {
            
            if (s->overrun_nonfatal) {
                av_log(h, AV_LOG_WARNING, "Circular buffer overrun. "
                        "Surviving due to overrun_nonfatal option\n");
                continue;
            } else {
                av_log(h, AV_LOG_ERROR, "Circular buffer overrun. "
                        "To avoid, increase fifo_size URL option. "
                        "To survive in such case, use overrun_nonfatal option\n");
                s->circular_buffer_error = AVERROR(EIO);
                goto end;
            }
        }
        av_fifo_generic_write(s->fifo, s->tmp, len+4, NULL);
        pthread_cond_signal(&s->cond);
    }

end:
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
    return NULL;
}

static void *circular_buffer_task_tx( void *_URLContext)
{
    URLContext *h = _URLContext;
    UDPContext *s = h->priv_data;
    int64_t target_timestamp = av_gettime_relative();
    int64_t start_timestamp = av_gettime_relative();
    int64_t sent_bits = 0;
    int64_t burst_interval = s->bitrate ? (s->burst_bits * 1000000 / s->bitrate) : 0;
    int64_t max_delay = s->bitrate ?  ((int64_t)h->max_packet_size * 8 * 1000000 / s->bitrate + 1) : 0;

    pthread_mutex_lock(&s->mutex);

    if (ff_socket_nonblock(s->udp_fd, 0) < 0) {
        av_log(h, AV_LOG_ERROR, "Failed to set blocking mode");
        s->circular_buffer_error = AVERROR(EIO);
        goto end;
    }

    for(;;) {
        int len;
        const uint8_t *p;
        uint8_t tmp[4];
        int64_t timestamp;

        len = av_fifo_size(s->fifo);

        while (len<4) {
            if (s->close_req)
                goto end;
            if (pthread_cond_wait(&s->cond, &s->mutex) < 0) {
                goto end;
            }
            len = av_fifo_size(s->fifo);
        }

        av_fifo_generic_read(s->fifo, tmp, 4, NULL);
        len = AV_RL32(tmp);

        av_assert0(len >= 0);
        av_assert0(len <= sizeof(s->tmp));

        av_fifo_generic_read(s->fifo, s->tmp, len, NULL);

        pthread_mutex_unlock(&s->mutex);

        if (s->bitrate) {
            timestamp = av_gettime_relative();
            if (timestamp < target_timestamp) {
                int64_t delay = target_timestamp - timestamp;
                if (delay > max_delay) {
                    delay = max_delay;
                    start_timestamp = timestamp + delay;
                    sent_bits = 0;
                }
                av_usleep(delay);
            } else {
                if (timestamp - burst_interval > target_timestamp) {
                    start_timestamp = timestamp - burst_interval;
                    sent_bits = 0;
                }
            }
            sent_bits += len * 8;
            target_timestamp = start_timestamp + sent_bits * 1000000 / s->bitrate;
        }

        p = s->tmp;
        while (len) {
            int ret;
            av_assert0(len > 0);
            if (!s->is_connected) {
                ret = sendto (s->udp_fd, p, len, 0,
                            (struct sockaddr *) &s->dest_addr,
                            s->dest_addr_len);
            } else
                ret = send(s->udp_fd, p, len, 0);
            if (ret >= 0) {
                len -= ret;
                p   += ret;
            } else {
                ret = ff_neterrno();
                if (ret != AVERROR(EAGAIN) && ret != AVERROR(EINTR)) {
                    pthread_mutex_lock(&s->mutex);
                    s->circular_buffer_error = ret;
                    pthread_mutex_unlock(&s->mutex);
                    return NULL;
                }
            }
        }

        pthread_mutex_lock(&s->mutex);
    }

end:
    pthread_mutex_unlock(&s->mutex);
    return NULL;
}

#endif



static int udp_open(URLContext *h, const char *uri, int flags)
{
    char hostname[1024], localaddr[1024] = "";
    int port, udp_fd = -1, tmp, bind_ret = -1, dscp = -1;
    UDPContext *s = h->priv_data;
    int is_output;
    const char *p;
    char buf[256];
    struct sockaddr_storage my_addr;
    int len;

    h->is_streamed = 1;

    is_output = !(flags & AVIO_FLAG_READ);
    if (s->buffer_size < 0)
        s->buffer_size = is_output ? UDP_TX_BUF_SIZE : UDP_RX_BUF_SIZE;

    if (s->sources) {
        if (ff_ip_parse_sources(h, s->sources, &s->filters) < 0)
            goto fail;
    }

    if (s->block) {
        if (ff_ip_parse_blocks(h, s->block, &s->filters) < 0)
            goto fail;
    }

    p = strchr(uri, '?');
    if (p) {
        if (av_find_info_tag(buf, sizeof(buf), "reuse", p)) {
            char *endptr = NULL;
            s->reuse_socket = strtol(buf, &endptr, 10);
            
            if (buf == endptr)
                s->reuse_socket = 1;
        }
        if (av_find_info_tag(buf, sizeof(buf), "overrun_nonfatal", p)) {
            char *endptr = NULL;
            s->overrun_nonfatal = strtol(buf, &endptr, 10);
            
            if (buf == endptr)
                s->overrun_nonfatal = 1;
            if (!HAVE_PTHREAD_CANCEL)
                av_log(h, AV_LOG_WARNING,
                       "'overrun_nonfatal' option was set but it is not supported "
                       "on this build (pthread support is required)\n");
        }
        if (av_find_info_tag(buf, sizeof(buf), "ttl", p)) {
            s->ttl = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "udplite_coverage", p)) {
            s->udplite_coverage = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "localport", p)) {
            s->local_port = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "pkt_size", p)) {
            s->pkt_size = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "buffer_size", p)) {
            s->buffer_size = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "connect", p)) {
            s->is_connected = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "dscp", p)) {
            dscp = strtol(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "fifo_size", p)) {
            s->circular_buffer_size = strtol(buf, NULL, 10);
            if (!HAVE_PTHREAD_CANCEL)
                av_log(h, AV_LOG_WARNING,
                       "'circular_buffer_size' option was set but it is not supported "
                       "on this build (pthread support is required)\n");
        }
        if (av_find_info_tag(buf, sizeof(buf), "bitrate", p)) {
            s->bitrate = strtoll(buf, NULL, 10);
            if (!HAVE_PTHREAD_CANCEL)
                av_log(h, AV_LOG_WARNING,
                       "'bitrate' option was set but it is not supported "
                       "on this build (pthread support is required)\n");
        }
        if (av_find_info_tag(buf, sizeof(buf), "burst_bits", p)) {
            s->burst_bits = strtoll(buf, NULL, 10);
        }
        if (av_find_info_tag(buf, sizeof(buf), "localaddr", p)) {
            av_strlcpy(localaddr, buf, sizeof(localaddr));
        }
        if (av_find_info_tag(buf, sizeof(buf), "sources", p)) {
            if (ff_ip_parse_sources(h, buf, &s->filters) < 0)
                goto fail;
        }
        if (av_find_info_tag(buf, sizeof(buf), "block", p)) {
            if (ff_ip_parse_blocks(h, buf, &s->filters) < 0)
                goto fail;
        }
        if (!is_output && av_find_info_tag(buf, sizeof(buf), "timeout", p))
            s->timeout = strtol(buf, NULL, 10);
        if (is_output && av_find_info_tag(buf, sizeof(buf), "broadcast", p))
            s->is_broadcast = strtol(buf, NULL, 10);
    }
    
    s->circular_buffer_size *= 188;
    if (flags & AVIO_FLAG_WRITE) {
        h->max_packet_size = s->pkt_size;
    } else {
        h->max_packet_size = UDP_MAX_PKT_SIZE;
    }
    h->rw_timeout = s->timeout;

    
    av_url_split(NULL, 0, NULL, 0, hostname, sizeof(hostname), &port, NULL, 0, uri);

    
    if (hostname[0] == '\0' || hostname[0] == '?') {
        
        if (!(flags & AVIO_FLAG_READ))
            goto fail;
    } else {
        if (ff_udp_set_remote_url(h, uri) < 0)
            goto fail;
    }

    if ((s->is_multicast || s->local_port <= 0) && (h->flags & AVIO_FLAG_READ))
        s->local_port = port;

    if (localaddr[0])
        udp_fd = udp_socket_create(h, &my_addr, &len, localaddr);
    else
        udp_fd = udp_socket_create(h, &my_addr, &len, s->localaddr);
    if (udp_fd < 0)
        goto fail;

    s->local_addr_storage=my_addr; //store for future multicast join

    
    if (s->reuse_socket > 0 || (s->is_multicast && s->reuse_socket < 0)) {
        s->reuse_socket = 1;
        if (setsockopt (udp_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&(s->reuse_socket), sizeof(s->reuse_socket)) != 0)
            goto fail;
    }

    if (s->is_broadcast) {
#ifdef SO_BROADCAST
        if (setsockopt (udp_fd, SOL_SOCKET, SO_BROADCAST, (const void*)&(s->is_broadcast), sizeof(s->is_broadcast)) != 0)
#endif
           goto fail;
    }

    
    if (s->udplite_coverage) {
        if (setsockopt (udp_fd, IPPROTO_UDPLITE, UDPLITE_SEND_CSCOV, (const void*)&(s->udplite_coverage), sizeof(s->udplite_coverage)) != 0)
            av_log(h, AV_LOG_WARNING, "socket option UDPLITE_SEND_CSCOV not available");

        if (setsockopt (udp_fd, IPPROTO_UDPLITE, UDPLITE_RECV_CSCOV, (const void*)&(s->udplite_coverage), sizeof(s->udplite_coverage)) != 0)
            av_log(h, AV_LOG_WARNING, "socket option UDPLITE_RECV_CSCOV not available");
    }

    if (dscp >= 0) {
        dscp <<= 2;
        if (setsockopt (udp_fd, IPPROTO_IP, 3, (const void*)&dscp, sizeof(dscp)) != 0)
            goto fail;
    }

    
    if (s->is_multicast && (h->flags & AVIO_FLAG_READ)) {
        bind_ret = bind(udp_fd,(struct sockaddr *)&s->dest_addr, len);
    }
    
    
    if (bind_ret < 0 && bind(udp_fd,(struct sockaddr *)&my_addr, len) < 0) {
        ff_log_net_error(h, AV_LOG_ERROR, "bind failed");
        goto fail;
    }

    len = sizeof(my_addr);
    getsockname(udp_fd, (struct sockaddr *)&my_addr, &len);
    s->local_port = udp_port(&my_addr, len);

    if (s->is_multicast) {
        if (h->flags & AVIO_FLAG_WRITE) {
            
            if (udp_set_multicast_ttl(udp_fd, s->ttl, (struct sockaddr *)&s->dest_addr) < 0)
                goto fail;
        }
        if (h->flags & AVIO_FLAG_READ) {
            
            if (s->filters.nb_include_addrs) {
                if (udp_set_multicast_sources(h, udp_fd,
                                              (struct sockaddr *)&s->dest_addr,
                                              s->dest_addr_len, &s->local_addr_storage,
                                              s->filters.include_addrs,
                                              s->filters.nb_include_addrs, 1) < 0)
                    goto fail;
            } else {
                if (udp_join_multicast_group(udp_fd, (struct sockaddr *)&s->dest_addr,(struct sockaddr *)&s->local_addr_storage) < 0)
                    goto fail;
            }
            if (s->filters.nb_exclude_addrs) {
                if (udp_set_multicast_sources(h, udp_fd,
                                              (struct sockaddr *)&s->dest_addr,
                                              s->dest_addr_len, &s->local_addr_storage,
                                              s->filters.exclude_addrs,
                                              s->filters.nb_exclude_addrs, 0) < 0)
                    goto fail;
            }
        }
    }

    if (is_output) {
        
        tmp = s->buffer_size;
        if (setsockopt(udp_fd, SOL_SOCKET, SO_SNDBUF, (const void*)&tmp, sizeof(tmp)) < 0) {
            ff_log_net_error(h, AV_LOG_ERROR, "setsockopt(SO_SNDBUF)");
            goto fail;
        }
    } else {
        
        tmp = s->buffer_size;
        if (setsockopt(udp_fd, SOL_SOCKET, SO_RCVBUF, (const void*)&tmp, sizeof(tmp)) < 0) {
            ff_log_net_error(h, AV_LOG_WARNING, "setsockopt(SO_RECVBUF)");
        }
        len = sizeof(tmp);
        if (getsockopt(udp_fd, SOL_SOCKET, SO_RCVBUF, (void*)&tmp, &len) < 0) {
            ff_log_net_error(h, AV_LOG_WARNING, "getsockopt(SO_RCVBUF)");
        } else {
            av_log(h, AV_LOG_DEBUG, "end receive buffer size reported is %d\n", tmp);
            if(tmp < s->buffer_size)
                av_log(h, AV_LOG_WARNING, "attempted to set receive buffer to size %d but it only ended up set as %d\n", s->buffer_size, tmp);
        }

        
        ff_socket_nonblock(udp_fd, 1);
    }
    if (s->is_connected) {
        if (connect(udp_fd, (struct sockaddr *) &s->dest_addr, s->dest_addr_len)) {
            ff_log_net_error(h, AV_LOG_ERROR, "connect");
            goto fail;
        }
    }

    s->udp_fd = udp_fd;

#if HAVE_PTHREAD_CANCEL
    

    if (is_output && s->bitrate && !s->circular_buffer_size) {
        
        av_log(h, AV_LOG_WARNING,"'bitrate' option was set but 'circular_buffer_size' is not, but required\n");
    }

    if ((!is_output && s->circular_buffer_size) || (is_output && s->bitrate && s->circular_buffer_size)) {
        int ret;

        
        s->fifo = av_fifo_alloc(s->circular_buffer_size);
        ret = pthread_mutex_init(&s->mutex, NULL);
        if (ret != 0) {
            av_log(h, AV_LOG_ERROR, "pthread_mutex_init failed : %s\n", strerror(ret));
            goto fail;
        }
        ret = pthread_cond_init(&s->cond, NULL);
        if (ret != 0) {
            av_log(h, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", strerror(ret));
            goto cond_fail;
        }
        ret = pthread_create(&s->circular_buffer_thread, NULL, is_output?circular_buffer_task_tx:circular_buffer_task_rx, h);
        if (ret != 0) {
            av_log(h, AV_LOG_ERROR, "pthread_create failed : %s\n", strerror(ret));
            goto thread_fail;
        }
        s->thread_started = 1;
    }
#endif

    return 0;
#if HAVE_PTHREAD_CANCEL
 thread_fail:
    pthread_cond_destroy(&s->cond);
 cond_fail:
    pthread_mutex_destroy(&s->mutex);
#endif
 fail:
    if (udp_fd >= 0)
        closesocket(udp_fd);
    av_fifo_freep(&s->fifo);
    ff_ip_reset_filters(&s->filters);
    return AVERROR(EIO);
}

static int udplite_open(URLContext *h, const char *uri, int flags)
{
    UDPContext *s = h->priv_data;

    // set default checksum coverage
    s->udplite_coverage = UDP_HEADER_SIZE;

    return udp_open(h, uri, flags);
}

static int udp_read(URLContext *h, uint8_t *buf, int size)
{
    UDPContext *s = h->priv_data;
    int ret;
    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
#if HAVE_PTHREAD_CANCEL
    int avail, nonblock = h->flags & AVIO_FLAG_NONBLOCK;

    if (s->fifo) {
        pthread_mutex_lock(&s->mutex);
        do {
            avail = av_fifo_size(s->fifo);
            if (avail) { // >=size) {
                uint8_t tmp[4];

                av_fifo_generic_read(s->fifo, tmp, 4, NULL);
                avail = AV_RL32(tmp);
                if(avail > size){
                    av_log(h, AV_LOG_WARNING, "Part of datagram lost due to insufficient buffer size\n");
                    avail = size;
                }

                av_fifo_generic_read(s->fifo, buf, avail, NULL);
                av_fifo_drain(s->fifo, AV_RL32(tmp) - avail);
                pthread_mutex_unlock(&s->mutex);
                return avail;
            } else if(s->circular_buffer_error){
                int err = s->circular_buffer_error;
                pthread_mutex_unlock(&s->mutex);
                return err;
            } else if(nonblock) {
                pthread_mutex_unlock(&s->mutex);
                return AVERROR(EAGAIN);
            } else {
                
                int64_t t = av_gettime() + 100000;
                struct timespec tv = { .tv_sec  =  t / 1000000,
                                       .tv_nsec = (t % 1000000) * 1000 };
                int err = pthread_cond_timedwait(&s->cond, &s->mutex, &tv);
                if (err) {
                    pthread_mutex_unlock(&s->mutex);
                    return AVERROR(err == ETIMEDOUT ? EAGAIN : err);
                }
                nonblock = 1;
            }
        } while(1);
    }
#endif

    if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
        ret = ff_network_wait_fd(s->udp_fd, 0);
        if (ret < 0)
            return ret;
    }
    ret = recvfrom(s->udp_fd, buf, size, 0, (struct sockaddr *)&addr, &addr_len);
    if (ret < 0)
        return ff_neterrno();
    if (ff_ip_check_source_lists(&addr, &s->filters))
        return AVERROR(EINTR);
    return ret;
}

static int udp_write(URLContext *h, const uint8_t *buf, int size)
{
    UDPContext *s = h->priv_data;
    int ret;

#if HAVE_PTHREAD_CANCEL
    if (s->fifo) {
        uint8_t tmp[4];

        pthread_mutex_lock(&s->mutex);

        
        if (s->circular_buffer_error<0) {
            int err = s->circular_buffer_error;
            pthread_mutex_unlock(&s->mutex);
            return err;
        }

        if(av_fifo_space(s->fifo) < size + 4) {
            
            pthread_mutex_unlock(&s->mutex);
            return AVERROR(ENOMEM);
        }
        AV_WL32(tmp, size);
        av_fifo_generic_write(s->fifo, tmp, 4, NULL); 
        av_fifo_generic_write(s->fifo, (uint8_t *)buf, size, NULL); 
        pthread_cond_signal(&s->cond);
        pthread_mutex_unlock(&s->mutex);
        return size;
    }
#endif
    if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
        ret = ff_network_wait_fd(s->udp_fd, 1);
        if (ret < 0)
            return ret;
    }

    if (!s->is_connected) {
        ret = sendto (s->udp_fd, buf, size, 0,
                      (struct sockaddr *) &s->dest_addr,
                      s->dest_addr_len);
    } else
        ret = send(s->udp_fd, buf, size, 0);

    return ret < 0 ? ff_neterrno() : ret;
}

static int udp_close(URLContext *h)
{
    UDPContext *s = h->priv_data;

#if HAVE_PTHREAD_CANCEL
    // Request close once writing is finished
    if (s->thread_started && !(h->flags & AVIO_FLAG_READ)) {
        pthread_mutex_lock(&s->mutex);
        s->close_req = 1;
        pthread_cond_signal(&s->cond);
        pthread_mutex_unlock(&s->mutex);
    }
#endif

    if (s->is_multicast && (h->flags & AVIO_FLAG_READ))
        udp_leave_multicast_group(s->udp_fd, (struct sockaddr *)&s->dest_addr,(struct sockaddr *)&s->local_addr_storage);
#if HAVE_PTHREAD_CANCEL
    if (s->thread_started) {
        int ret;
        // Cancel only read, as write has been signaled as success to the user
        if (h->flags & AVIO_FLAG_READ) {
#ifdef _WIN32
            
            shutdown(s->udp_fd, SD_RECEIVE);
            CancelIoEx((HANDLE)(SOCKET)s->udp_fd, NULL);
#else
            pthread_cancel(s->circular_buffer_thread);
#endif
        }
        ret = pthread_join(s->circular_buffer_thread, NULL);
        if (ret != 0)
            av_log(h, AV_LOG_ERROR, "pthread_join(): %s\n", strerror(ret));
        pthread_mutex_destroy(&s->mutex);
        pthread_cond_destroy(&s->cond);
    }
#endif
    closesocket(s->udp_fd);
    av_fifo_freep(&s->fifo);
    ff_ip_reset_filters(&s->filters);
    return 0;
}

const URLProtocol ff_udp_protocol = {
    .name                = "udp",
    .url_open            = udp_open,
    .url_read            = udp_read,
    .url_write           = udp_write,
    .url_close           = udp_close,
    .url_get_file_handle = udp_get_file_handle,
    .priv_data_size      = sizeof(UDPContext),
    .priv_data_class     = &udp_class,
    .flags               = URL_PROTOCOL_FLAG_NETWORK,
};

const URLProtocol ff_udplite_protocol = {
    .name                = "udplite",
    .url_open            = udplite_open,
    .url_read            = udp_read,
    .url_write           = udp_write,
    .url_close           = udp_close,
    .url_get_file_handle = udp_get_file_handle,
    .priv_data_size      = sizeof(UDPContext),
    .priv_data_class     = &udplite_context_class,
    .flags               = URL_PROTOCOL_FLAG_NETWORK,
};
