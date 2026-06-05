#!/bin/bash
set -e

CURDIR=$(dirname $0)
ICONB64=$(base64 -w0 ${CURDIR}/res/favicon.ico)

gendeerheader() {
    DEERB64=$(base64 -w0 ${CURDIR}/res/deer_$1.gif)

    cat << EOF | xxd -i -n deer_$1 > deer_$1.c
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

        img {
            max-width: 90%;
            max-height: 90vh;
            object-fit: contain;
        } </style>
    <title>deer status</title>
  </head>
  <body>
    <img alt='deer' src='data:image/gif;base64,${DEERB64}'/>
  </body>
</html>
EOF
}

gendeerheader $1
