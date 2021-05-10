/* Taken from https://www.w3.org/Library/src/HTParse.html */

#include "htparse.h"

#include <string.h>
#include <ctype.h>

void scan (char* raw_uri, Uri* uri)
{
    char* p;
    char* after_scheme = raw_uri;
    memset(uri, '\0', sizeof(Uri));

    /* Look for fragment identifier */
    if ((p = strchr(raw_uri, '#')) != NULL) {
        *p++ = '\0';
        uri->fragment = p;
    }


    if ((p = strchr(raw_uri, ' ')) != NULL) *p++ = '\0';

    for(p=raw_uri; *p; p++) {

        /*
        ** Look for any whitespace. This is very bad for pipelining as it
        ** makes the request invalid
        */
        if (isspace((int) *p)) {
            char *orig=p, *dest=p+1;
            while ((*orig++ = *dest++));
            p = p-1;
        }
        if (*p=='/' || *p=='#' || *p=='?')
            break;
        if (*p==':') {
            *p = 0;
            uri->scheme = after_scheme; /* Scheme has been specified */

/* The combination of gcc, the "-O" flag and the HP platform is
   unhealthy. The following three lines is a quick & dirty fix, but is
   not recommended. Rather, turn off "-O". */

/*		after_scheme = p;*/
/*		while (*after_scheme == 0)*/
/*		    after_scheme++;*/

            after_scheme = p + 1;

            if (0==strcasecmp("URL", uri->scheme)) {
                uri->scheme = NULL;  /* Ignore IETF's URL: pre-prefix */
            } else break;
        }
    }

    p = after_scheme;
    if (*p=='/'){
        if (p[1]=='/') {
            uri->host = p + 2;		/* host has been specified 	*/
            *p=0;			/* Terminate access 		*/
            p=strchr(uri->host, '/');	/* look for end of host raw_uri if any */
            if(p) {
                *p=0;			/* Terminate host */
                uri->absolute = p + 1;		/* Root has been found */
            }
        } else {
            uri->absolute = p + 1;		/* Root found but no host */
        }
    } else {
        uri->relative = (*after_scheme) ? after_scheme : 0; /* zero for "" */
    }
}
