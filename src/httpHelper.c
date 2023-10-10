#include "../includes/essentials.h"

char *getHttpStatusText(http_code code)
{
    switch (code)
    {
    case HTTP_OK:
        return "200 OK";
    case 400:
        return "400 Bad Request";
    case 401:
        return "401 Unauthorized";
    case 402:
        return "402 Payment Required";
    case 403:
        return "403 Forbidden";
    case 404:
        return "404 Not Found";
    case 405:
        return "405 Method Not Allowed";
    case 406:
        return "406 Not Acceptable";
    case 407:
        return "407 Proxy Authentication Required";
    case 408:
        return "408 Request Timeout";
    case 409:
        return "409 Conflict";
    case 410:
        return "410 Gone";
    case 411:
        return "411 Length Required";
    case 412:
        return "412 Precondition Failed";
    case 413:
        return "413 Request Entity Too Large";
    case 414:
        return "414 Request-URI Too Long";
    case 415:
        return "415 Unsupported Media Type";
    case 416:
        return "416 Requested Range Not Satisfiable";
    case 417:
        return "417 Expectation Failed";
    case 418:
        return "418 I'm a teapot (RFC 2324)";
    case 420:
        return "420 Enhance Your Calm (Twitter)";
    case 422:
        return "422 Unprocessable Entity (WebDAV)";
    case 423:
        return "423 Locked (WebDAV)";
    case 424:
        return "424 Failed Dependency (WebDAV)";
    case 425:
        return "425 Reserved for WebDAV";
    case 426:
        return "426 Upgrade Required";
    case 428:
        return "428 Precondition Required";
    case 429:
        return "429 Too Many Requests";
    case 431:
        return "431 Request Header Fields Too Large";
    case 444:
        return "444 No Response (Nginx)";
    case 449:
        return "449 Retry With (Microsoft)";
    case 450:
        return "450 Blocked by Windows Parental Controls (Microsoft)";
    case 451:
        return "451 Unavailable For Legal Reasons";
    case 499:
        return "499 Client Closed Request (Nginx)";
    case 500:
        return "500 Internal Server Error";
    case 501:
        return "501 Not Implemented";
    case 502:
        return "502 Bad Gateway";
    case 503:
        return "503 Service Unavailable";
    case 504:
        return "504 Gateway Timeout";
    case 505:
        return "505 HTTP Version Not Supported";
    case 506:
        return "506 Variant Also Negotiates (Experimental)";
    case 507:
        return "507 Insufficient Storage (WebDAV)";
    case 508:
        return "508 Loop Detected (WebDAV)";
    case 509:
        return "509 Bandwidth Limit Exceeded (Apache)";
    case 510:
        return "510 Not Extended";
    case 511:
        return "511 Network Authentication Required";
    case 598:
        return "598 Network read timeout error";
    case 599:
        return "599 Network connect timeout error";
    default:
        return "Código não tratado.";
    }
}

http_code httpReqText2Number(const char *reqText)
{
    if (!strcmp(reqText, "GET"))
        return HTTP_GET;
    else if (!strcmp(reqText, "HEAD"))
        return HTTP_HEAD;
    else if (!strcmp(reqText, "OPTIONS"))
        return HTTP_OPTIONS;
    else if (!strcmp(reqText, "TRACE"))
        return HTTP_TRACE;
    else
        return -1;
}