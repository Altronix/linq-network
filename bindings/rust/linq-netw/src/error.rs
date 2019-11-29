extern crate linq_netw_sys;
use std::error;
use std::fmt;

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum NetworkErrorKind {
    Ok,
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
    Unknown,
}

#[derive(Debug, Clone)]
pub struct NetworkError {
    pub kind: NetworkErrorKind,
}

impl NetworkError {
    pub fn to_http(&self) -> u16 {
        match self.kind {
            NetworkErrorKind::DeviceNotFound => 404,
            NetworkErrorKind::HttpBadRequest => 400,
            NetworkErrorKind::HttpUnauthorized => 403,
            NetworkErrorKind::HttpNotFound => 404,
            NetworkErrorKind::HttpServerError => 500,
            NetworkErrorKind::HttpTryAgainLater => 504,
            _ => 500,
        }
    }
}

impl fmt::Display for NetworkError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use NetworkErrorKind::*;
        match self.kind {
            Ok => write!(f, "Ok!"),
            OutOfMemory => write!(f, "System out of memory"),
            BadArgs => write!(f, "Caller provided bad arguments"),
            Protocol => write!(f, "Network protocol error"),
            Io => write!(f, "System IO error"),
            DeviceNotFound => write!(f, "Device not found"),
            Timeout => write!(f, "Request timeout"),
            ShuttingDown => write!(f, "System is shutting down"),
            HttpBadRequest => write!(f, "HTTP (500) Bad request"),
            HttpUnauthorized => write!(f, "HTTP (403) Unauthorized"),
            HttpNotFound => write!(f, "HTTP (404) Not found"),
            HttpServerError => write!(f, "HTTP (500) Server error"),
            HttpTryAgainLater => write!(f, "HTTP (504) Try again later"),
            Unknown => write!(f, "Impossible error"),
        }
    }
}

impl error::Error for NetworkError {
    fn source(&self) -> Option<&(dyn error::Error + 'static)> {
        match self.kind {
            _ => Some(self),
        }
    }
}

impl From<linq_netw_sys::E_LINQ_ERROR> for NetworkError {
    fn from(err: linq_netw_sys::E_LINQ_ERROR) -> NetworkError {
        use linq_netw_sys::*;
        match err {
            E_LINQ_ERROR_LINQ_ERROR_OK => NetworkError {
                kind: NetworkErrorKind::Ok,
            },
            E_LINQ_ERROR_LINQ_ERROR_OOM => NetworkError {
                kind: NetworkErrorKind::OutOfMemory,
            },
            E_LINQ_ERROR_LINQ_ERROR_BAD_ARGS => NetworkError {
                kind: NetworkErrorKind::BadArgs,
            },
            E_LINQ_ERROR_LINQ_ERROR_PROTOCOL => NetworkError {
                kind: NetworkErrorKind::Protocol,
            },
            E_LINQ_ERROR_LINQ_ERROR_IO => NetworkError {
                kind: NetworkErrorKind::Io,
            },
            E_LINQ_ERROR_LINQ_ERROR_DEVICE_NOT_FOUND => NetworkError {
                kind: NetworkErrorKind::DeviceNotFound,
            },
            E_LINQ_ERROR_LINQ_ERROR_TIMEOUT => NetworkError {
                kind: NetworkErrorKind::Timeout,
            },
            E_LINQ_ERROR_LINQ_ERROR_SHUTTING_DOWN => NetworkError {
                kind: NetworkErrorKind::ShuttingDown,
            },
            E_LINQ_ERROR_LINQ_ERROR_400 => NetworkError {
                kind: NetworkErrorKind::HttpBadRequest,
            },
            E_LINQ_ERROR_LINQ_ERROR_403 => NetworkError {
                kind: NetworkErrorKind::HttpUnauthorized,
            },
            E_LINQ_ERROR_LINQ_ERROR_404 => NetworkError {
                kind: NetworkErrorKind::HttpNotFound,
            },
            E_LINQ_ERROR_LINQ_ERROR_500 => NetworkError {
                kind: NetworkErrorKind::HttpServerError,
            },
            E_LINQ_ERROR_LINQ_ERROR_504 => NetworkError {
                kind: NetworkErrorKind::HttpTryAgainLater,
            },
            _ => NetworkError {
                kind: NetworkErrorKind::Unknown,
            },
        }
    }
}
