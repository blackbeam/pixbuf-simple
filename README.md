## pixbuf-simple
Simple synchronous js interface to gdk-pixbuf 2.0

### Requirements:
1. gdk-pixbuf 2.0

### Installation:
```bash
npm install pixbuf-simple
```

### Usage example:
```javascript
var p = require('pixbuf-simple');

// Get list of supported formats
p.formats;

// New pixbuf
var pb = new Pixbuf(/*has alpha*/ false, /*width*/ 2, /*height*/ 2);
console.log(pb);
// > { '0': { r: xx, g: xx, b: xx },
// >   '1': { r: xx, g: xx, b: xx },
// >   '2': { r: xx, g: xx, b: xx },
// >   '3': { r: xx, g: xx, b: xx },
// >   length: 4,
// >   height: 2,
// >   width: 2,
// >   has_alpha: false,
// >   pixels: <SlowBuffer xx xx xx xx xx xx xx ..> }

// Set pixel value
pb[3] = {r: 0, g: 255, b: 0};

// New pixbuf from buffer of pixels
var npb = new Pixbuf(pb.pixels, pb.has_alpha, pb.width, pb.height);

// New pixbuf from file
var fpb = new Pixbuf('path/to/file.jpeg');

// New pixbuf from file at scale (refers to gdk_pixbuf_new_from_file_at_scale)
fpb = new Pixbuf('path/to/file.jpeg', /*width*/ 10, /*height*/ 20, /*preserve_aspect_ratio*/ false);

// Encode pixbuf
var raw_image_buffer = fpb.toImage('png', {complression: '9'});
```

### Additional info:

  For list of supported image formats and options please refer to gdk-pixbuf 2.0 documentation.
  Gdk-pixbuf has limited support of transparency.
