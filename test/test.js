var Pixbuf = require( '../build/default/pixbuf' ).Pixbuf;

module.exports = {
  creation : {
	'New without alpha' : function ( test ) {
	  test.expect ( 6 );
	  var p = new Pixbuf( false, 10 , 10 );
	  test.equal ( p.length, 100 );
	  test.equal ( p.width, 10 );
	  test.equal ( p.height, 10 );
	  test.equal ( p.has_alpha, false );
	  test.ok ( Buffer.isBuffer( p.pixels ) );
	  test.equal ( p.pixels.length, 320 );
	  test.done();
	},
	'New with alpha' : function ( test ) {
	  test.expect( 6 );
	  var p = new Pixbuf( true, 10 , 10 );
	  test.equal ( p.length, 100 );
	  test.equal ( p.width, 10 );
	  test.equal ( p.height, 10 );
	  test.equal ( p.has_alpha, true );
	  test.ok ( Buffer.isBuffer( p.pixels ) );
	  test.equal ( p.pixels.length, 400 );
	  test.done();
	},
	'New from buffer without alpha' : function ( test ) {
	  test.expect( 10 );
	  var buf = new Buffer( [0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x92, 0x09, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x74, 0x6f] );
	  var p = new Pixbuf ( buf, false, 2, 2 );
	  test.equal ( p.length, 4 );
	  test.equal ( p.width, 2 );
	  test.equal ( p.height, 2 );
	  test.equal ( p.has_alpha, false );
	  test.ok ( Buffer.isBuffer( p.pixels ) );
	  test.equal ( p.pixels.length, 16 );
	  test.deepEqual ( p[0], { r : 0, g : 0, b : 0} );
	  test.deepEqual ( p[1], { r : 255, g : 0, b : 0} );
	  test.deepEqual ( p[2], { r : 0, g : 255, b : 0} );
	  test.deepEqual ( p[3], { r : 0, g : 0, b : 255} );
	  test.done();
	},
	'New from buffer with alpha' : function ( test ) {
	  test.expect( 10 );
	  var buf = new Buffer( [0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x64, 0x00, 0xff, 0x00, 0xc8, 0x00, 0x00, 0xff, 0xff] );
	  var p = new Pixbuf ( buf, true, 2, 2 );
	  test.equal ( p.length, 4 );
	  test.equal ( p.width, 2 );
	  test.equal ( p.height, 2 );
	  test.equal ( p.has_alpha, true );
	  test.ok ( Buffer.isBuffer( p.pixels ) );
	  test.equal ( p.pixels.length, 16 );
	  test.deepEqual ( p[0], { r : 0, g : 0, b : 0, a : 0} );
	  test.deepEqual ( p[1], { r : 255, g : 0, b : 0, a : 100} );
	  test.deepEqual ( p[2], { r : 0, g : 255, b : 0, a : 200} );
	  test.deepEqual ( p[3], { r : 0, g : 0, b : 255, a : 255} );
	  test.done();
	},
	'New from file' : function ( test ) {
	  test.expect ( 10 );
	  var p = new Pixbuf ( 'test/fixtures/img.png' );
	  test.equal ( p.length, 4 );
	  test.equal ( p.width, 2 );
	  test.equal ( p.height, 2 );
	  test.equal ( p.has_alpha, false );
	  test.ok ( Buffer.isBuffer( p.pixels ) );
	  test.equal ( p.pixels.length, 16 );
	  test.deepEqual ( p[0], { r : 0, g : 0, b : 0} );
	  test.deepEqual ( p[1], { r : 255, g : 0, b : 0} );
	  test.deepEqual ( p[2], { r : 0, g : 255, b : 0} );
	  test.deepEqual ( p[3], { r : 0, g : 0, b : 255} );
	  test.done();
	}
  },
  manipulation : {
	'Set pixel' : function ( test ) {
	  var p = new Pixbuf( false, 1 , 1 );
	  test.deepEqual ( p[0] = { r : 0, g : 0, b : 0 }, { r : 0, g : 0, b : 0 } );
	  test.deepEqual ( p[0], { r : 0, g : 0, b : 0 } );
	  test.done();
	}
  },
  conversion : {
	'To image' : function ( test ) {
	  var p = new Pixbuf( false, 1 , 1 );
	  p[0] = { r : 0, g : 0, b : 0 };
	  var img = p.toImage( 'png', { 'compression' : '9' } );
	  require('fs').writeFile( 'test/tmp.png', img, 'ascii',
		function( err ) {
		  if ( err ) throw err;
		  var p2 = new Pixbuf ( 'test/tmp.png' );
		  require('fs').unlinkSync( 'test/tmp.png' );
		  test.deepEqual ( p[0], { r : 0, g : 0, b : 0 } );
		  test.done();
		}
	  );
	}
  }
};
