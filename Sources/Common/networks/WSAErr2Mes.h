#ifndef WSAERR2MES 
#define WSAERR2MES 

inline const char* LastWSAErr2Mes(int errNo)
{
    static char bufferErr[1024];
	switch (errNo)
	{
        case WSAEINTR:          return "Interrupted system call.";
	    case WSAEBADF:          return "Bad file number.";
	    case WSAEACCES:         return "Permission denied.";
	    case WSAEFAULT:         return "Bad address.";
	    case WSAEINVAL:         return "Invalid argument.";
	    case WSAEMFILE:         return "Too many open files.";
	    case WSAEWOULDBLOCK:    return "Operation would block.";
	    case WSAEINPROGRESS:    return "Operation now in progress. This error is returned if any Windows Sockets API function is called while a blocking function is in progress.";
	    case WSAEALREADY:       return "Operation already in progress.";
	    case WSAENOTSOCK:       return "Socket operation on nonsocket.";
	    case WSAEDESTADDRREQ:   return "Destination address required.";
	    case WSAEMSGSIZE:       return "Message too long.";
	    case WSAEPROTOTYPE:     return "Protocol wrong type for socket.";
	    case WSAENOPROTOOPT:    return "Protocol not available.";
	    case WSAEPROTONOSUPPORT: return "Protocol not supported.";
	    case WSAESOCKTNOSUPPORT: return "Socket type not supported.";
	    case WSAEOPNOTSUPP:     return "Operation not supported on socket.";
	    case WSAEPFNOSUPPORT:   return "Protocol family not supported.";
	    case WSAEAFNOSUPPORT:   return "Address family not supported by protocol family.";
	    case WSAEADDRINUSE:     return "Address already in use.";
	    case WSAEADDRNOTAVAIL:  return "Cannot assign requested address.";
	    case WSAENETDOWN:       return "Network is down. This error may be reported at any time if the Windows Sockets implementation detects an underlying failure.";
	    case WSAENETUNREACH:    return "Network is unreachable.";
	    case WSAENETRESET:      return "Network dropped connection on reset.";
	    case WSAECONNABORTED:   return "Software caused connection abort.";
	    case WSAECONNRESET:     return "Connection reset by peer.";
	    case WSAENOBUFS:        return "No buffer space available.";
	    case WSAEISCONN:        return "Socket is already connected.";
	    case WSAENOTCONN:       return "Socket is not connected.";
	    case WSAESHUTDOWN:      return "Cannot send after socket shutdown.";
	    case WSAETOOMANYREFS:   return "Too many references: cannot splice.";
	    case WSAETIMEDOUT:      return "Connection timed out.";
	    case WSAECONNREFUSED:   return "Connection refused.";
	    case WSAELOOP:          return "Too many levels of symbolic links.";
	    case WSAENAMETOOLONG:   return "File name too long.";
	    case WSAEHOSTDOWN:      return "Host is down.";
	    case WSAEHOSTUNREACH:   return "No route to host.";
	    case WSASYSNOTREADY:    return "Returned by WSAStartup(), indicating that the network subsystem is unusable.";
	    case WSAVERNOTSUPPORTED: return "Returned by WSAStartup(), indicating that the Windows Sockets DLL cannot support this application.";
	    case WSANOTINITIALISED: return "Winsock not initialized. This message is returned by any function except WSAStartup(), indicating that a successful WSAStartup() has not yet been performed.";
	    case WSAEDISCON:        return "Disconnect.";
	    case WSAHOST_NOT_FOUND: return "Host not found. This message indicates that the key (name, address, and so on) was not found.";
	    case WSATRY_AGAIN:      return "Nonauthoritative host not found. This error may suggest that the name service itself is not functioning.";
	    case WSANO_RECOVERY:    return "Nonrecoverable error. This error may suggest that the name service itself is not functioning.";
	    case WSANO_DATA:        return "Valid name, no data record of requested type. This error indicates that the key (name, address, and so on) was not found.";
        default:
            sprintf(bufferErr,"Unknown error 0x%x(%d)",errNo,errNo);
            return bufferErr;
    }
}

#endif