use std::error;
use std::fmt;

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum ErrorKind {
    OutOfMemory,
    BadArgs,
    Protocol,
    Io,
    DeviceNotFound,
    Timeout,
    ShuttingDown,
    HttpBadRequest,
    HttpUnauthorized,
    HttpNotFound,
    HttpServerError,
    HttpTryAgainLater,
}

#[derive(Debug, Clone)]
struct Error {
    kind: ErrorKind,
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.kind {
            ErrorKind::OutOfMemory => write!(f, "System out of memory"),
            ErrorKind::BadArgs => write!(f, "Caller provided bad arguments"),
            ErrorKind::Protocol => write!(f, "Network protocol error"),
            ErrorKind::Io => write!(f, "System IO error"),
            ErrorKind::DeviceNotFound => write!(f, "Device not found"),
            ErrorKind::Timeout => write!(f, "Request timeout"),
            ErrorKind::ShuttingDown => write!(f, "System is shutting down"),
            ErrorKind::HttpBadRequest => write!(f, "HTTP (500) Bad request"),
            ErrorKind::HttpUnauthorized => write!(f, "HTTP (403) Unauthorized"),
            ErrorKind::HttpNotFound => write!(f, "HTTP (404) Not found"),
            ErrorKind::HttpServerError => write!(f, "HTTP (500) Server error"),
            ErrorKind::HttpTryAgainLater => {
                write!(f, "HTTP (504) Try again later")
            }
        }
    }
}

impl error::Error for Error {
    fn source(&self) -> Option<&(dyn error::Error + 'static)> {
        match self.kind {
            _ => Some(self),
        }
    }
}
