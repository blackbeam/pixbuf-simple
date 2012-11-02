var Pixbuf = require( '../build/Release/pixbuf' ).Pixbuf;

module.exports = {
    creation : {
        'New without alpha' : function (test) {
            test.expect(6);
            var p = new Pixbuf(false, 10 , 10);
            test.equal(p.length, 100);
            test.equal(p.width, 10);
            test.equal(p.height, 10);
            test.equal(p.has_alpha, false);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 320);
            test.done();
        },
        'New with alpha' : function (test) {
            test.expect(6);
            var p = new Pixbuf(true, 10 , 10);
            test.equal(p.length, 100);
            test.equal(p.width, 10);
            test.equal(p.height, 10);
            test.equal(p.has_alpha, true);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 400);
            test.done();
        },
        'New from buffer without alpha' : function (test) {
            test.expect(10);
            var buf = new Buffer([0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x92, 0x09, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x74, 0x6f]);
            var p = new Pixbuf(buf, false, 2, 2);
            test.equal(p.length, 4);
            test.equal(p.width, 2);
            test.equal(p.height, 2);
            test.equal(p.has_alpha, false);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 16);
            test.deepEqual(p[0], { r : 0, g : 0, b : 0 });
            test.deepEqual(p[1], { r : 255, g : 0, b : 0 });
            test.deepEqual(p[2], { r : 0, g : 255, b : 0 });
            test.deepEqual(p[3], { r : 0, g : 0, b : 255 });
            test.done();
        },
        'New from buffer with alpha' : function (test) {
            test.expect(10);
            var buf = new Buffer([0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x64, 0x00, 0xff, 0x00, 0xc8, 0x00, 0x00, 0xff, 0xff]);
            var p = new Pixbuf(buf, true, 2, 2);
            test.equal(p.length, 4);
            test.equal(p.width, 2);
            test.equal(p.height, 2);
            test.equal(p.has_alpha, true);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 16);
            test.deepEqual(p[0], { r : 0, g : 0, b : 0, a : 0 });
            test.deepEqual(p[1], { r : 255, g : 0, b : 0, a : 100 });
            test.deepEqual(p[2], { r : 0, g : 255, b : 0, a : 200 });
            test.deepEqual(p[3], { r : 0, g : 0, b : 255, a : 255 });
            test.done();
        },
        'New from file' : function (test) {
            test.expect(10);
            var p = new Pixbuf('test/fixtures/img.png');
            test.equal(p.length, 4);
            test.equal(p.width, 2);
            test.equal(p.height, 2);
            test.equal(p.has_alpha, false);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 16);
            test.deepEqual(p[0], { r : 0, g : 0, b : 0 });
            test.deepEqual(p[1], { r : 255, g : 0, b : 0 });
            test.deepEqual(p[2], { r : 0, g : 255, b : 0 });
            test.deepEqual(p[3], { r : 0, g : 0, b : 255 });
            test.done();
        },
        'New from file at scale' : function (test) {
            test.expect(22);
            var p = new Pixbuf('test/fixtures/img.png', 4, 4);
            test.equal(p.length, 16);
            test.equal(p.width, 4);
            test.equal(p.height, 4);
            test.equal(p.has_alpha, false);
            test.ok(Buffer.isBuffer(p.pixels));
            test.equal(p.pixels.length, 48);
            test.deepEqual(p[0x00], { r : 0x00, g : 0x00, b : 0x00 });
            test.deepEqual(p[0x01], { r : 0x40, g : 0x00, b : 0x00 });
            test.deepEqual(p[0x02], { r : 0xbf, g : 0x00, b : 0x00 });
            test.deepEqual(p[0x03], { r : 0xff, g : 0x00, b : 0x00 });
            test.deepEqual(p[0x04], { r : 0x00, g : 0x40, b : 0x00 });
            test.deepEqual(p[0x05], { r : 0x30, g : 0x30, b : 0x10 });
            test.deepEqual(p[0x06], { r : 0x8f, g : 0x10, b : 0x30 });
            test.deepEqual(p[0x07], { r : 0xbf, g : 0x00, b : 0x40 });
            test.deepEqual(p[0x08], { r : 0x00, g : 0xbf, b : 0x00 });
            test.deepEqual(p[0x09], { r : 0x10, g : 0x8f, b : 0x30 });
            test.deepEqual(p[0x0a], { r : 0x30, g : 0x30, b : 0x8f });
            test.deepEqual(p[0x0b], { r : 0x40, g : 0x00, b : 0xbf });
            test.deepEqual(p[0x0c], { r : 0x00, g : 0xff, b : 0x00 });
            test.deepEqual(p[0x0d], { r : 0x00, g : 0xbf, b : 0x40 });
            test.deepEqual(p[0x0e], { r : 0x00, g : 0x40, b : 0xbf });
            test.deepEqual(p[0x0f], { r : 0x00, g : 0x00, b : 0xff });
            test.done();
        }
    },
    manipulation : {
        'Set pixel' : function (test) {
            var p = new Pixbuf(false, 1 , 1);
            test.deepEqual(p[0] = { r : 0, g : 0, b : 0 }, { r : 0, g : 0, b : 0 });
            test.deepEqual(p[0], { r : 0, g : 0, b : 0 });
            test.done();
        },
        'Draw glyph syncroniously': function (test) {
            var p = new Pixbuf(false, 10, 10);
            for (var i = 0; i < 100; i++) {
                p[i] = {r: 255, g: 255, b: 255};
            }
            p.drawGlyph({x1:1,x2:5,y1:1,y2:5}, {r:100,g:255,b:100,a:127});
            test.deepEqual(p[12], {r: 177, g: 255, b:177});
            test.done();
        },
        'Draw glyph asyncroniously': function (test) {
            var p = new Pixbuf(false, 10, 10);
            for (var i = 0; i < 100; i++) {
                p[i] = {r: 255, g: 255, b: 255};
            }
            p.drawGlyph({x1:0,x2:5,y1:0,y2:5}, {r:100,g:255,b:100,a:127}, function (err) {
                test.equal(err, null);
                test.deepEqual(p[0], {r: 177, g: 255, b:177});
                test.done();
            });
        }
    },
    conversion : {
        'To image asyncroniously' : function (test) {
            var p = new Pixbuf(false, 1 , 1);
            p[0] = { r : 64, g : 54, b : 44 };
            p.toImage('png', {'compression': '9'}, function (err, img) {
                if (err) throw err;
                require('fs').writeFile('test/tmp.png', img, 'ascii', function (err) {
                    if (err) throw err;
                    var p2 = new Pixbuf('test/tmp.png');
                    require('fs').unlinkSync('test/tmp.png');
                    test.deepEqual(p[0], { r: 64, g: 54, b: 44});
                    test.done();
                });
            });
        },
        'To image syncroniously': function (test) {
            var p = new Pixbuf(false, 1, 1);
            p[0] = {r: 72, g: 82, b: 92};
            var img = p.toImage('png', {'compression': '9'});
            require('fs').writeFile('test/tmp.png', img, 'ascii', function (err) {
                if (err) throw err;
                var p2 = new Pixbuf('test/tmp.png');
                require('fs').unlinkSync('test/tmp.png');
                test.deepEqual(p[0], {r: 72, g: 82, b: 92});
                test.done();
            });
        },
        'Scale' : function (test) {
            test.expect(5);
            var p = new Pixbuf(false, 100, 100);
            p.scale(50, 50);
            test.equal(p.length, 50 * 50);
            test.equal(p.width, 50);
            test.equal(p.height, 50);
            test.equal(p.has_alpha, false );
            test.ok(Buffer.isBuffer(p.pixels));
            test.done();
        }
    }
};
