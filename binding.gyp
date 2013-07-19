{
    "targets": [
        {
            "target_name": "pixbuf",
            "sources": [
                "src/pixbuf.cc"
            ],
            "libraries": [
                "<!@(pkg-config --libs gdk-pixbuf-2.0)"
            ],
            "cflags": [
                "<!@(pkg-config --cflags gdk-pixbuf-2.0)"
            ]
        }
    ]
}