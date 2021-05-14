# my_curl

## <a name="description"></a>Description

This is a partial replica of the Unix `curl` command. Just like its counterpart, `my_curl` is used to fetch data from a server without any user interaction.

The purpose of this project was first and foremost to explore network communication, the HTTP protocol and the direct use of network sockets. For this reason, `my_curl` only supports the HTTP protocol, and none of the numerous `curl` options (-i, -I, -v, -s, etc.) are implemented. Also, `my_curl` does not support [transfer encoding](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding).

## Compiling

To compile the application, run `make` from the project's root directory.

## Running the command

After the command has compiled, run the following command : `my_curl <URL>`. Because of the limitations set forth in the [Description](#description) section, many URLs will not produce any output, including the ones where the response body is empty. Generally speaking, there is often no response body for responses with codes 1xx, 201, 204, 205, 3xx, 4xx and 5xx.

Here are some commands you can use to see `my_curl` in action:

- `./my_curl example.com`
- `./my_curl www.w3.org`
- `./my_curl www.w3.org/Protocols/rfc2616/rfc2616.html`
- `./my_curl wiki.python.org`
- `./my_curl developer.mozilla.org`
- `./my_curl news.ycombinator.com`
- `./my_curl apple.ca`
- `./my_curl man7.org/linux/man-pages/man1/curl.1.html`

## Implementation notes

Perhaps one of the most interesting implementation choices is the use of synchronous I/O multiplexing when transferring the HTTP response from the socket to the standard output. See the `print_chunk` function in src/buffered_response_printer.c and the documentation for [the `select` function](https://man7.org/linux/man-pages/man3/pselect.3p.html).