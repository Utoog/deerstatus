#!/bin/bash
set -e

CURDIR=$(dirname $0)
ICONB64=$(base64 -w0 ${CURDIR}/res/favicon.ico)

gendeerheader() {
    DEER=$(cat ${CURDIR}/res/deer_$1.txt)

    cat << EOF | xxd -i -n deer_$1 > deer_$1_light.c
<!DOCTYPE html>
<html>
  <head>
    <link rel='icon' type='image/png' href='data:image/png;base64,${ICONB64}'/>
    <style>body {
            background-color: black;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }

        .ascii {
            color: white;
            font-family: monospace;
            white-space: pre;
            # text-align: center;
            # padding: 1rem;
            # border-radius: 8px;
            max-width: 90%;
            overflow-x: auto;
            # line-height: 1.2; /* Tighter line spacing for ASCII art */
            # font-size: 14px;
        } </style>
    <title>deer status</title>
  </head>
  <body>
    <div class='ascii'>
${DEER}
    </div>
  </body>
</html>
EOF
}

gendeerheader $1
