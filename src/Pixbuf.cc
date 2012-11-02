#include "Pixbuf.hh"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

#include <cstring> /* memcpy */
#include <cstdlib>

using namespace node;
using namespace v8;

Persistent<FunctionTemplate> Pixbuf::constructor_template;
static Persistent<String> red_sym;
static Persistent<String> green_sym;
static Persistent<String> blue_sym;
static Persistent<String> alpha_sym;

struct render_work_t
{
    Pixbuf *src;
    uv_work_t request;
    const char *type;
    gchar **keys;
    gchar **values;
    uint32_t optc;
    GError *err;
    Persistent<Function> cb;
    gchar *buffer;
    gsize buffer_size;
    gboolean isOk;
};

struct draw_glyph_work_t
{
    Pixbuf *src;
    uv_work_t request;
    const char *error;
    int x1;
    int y1;
    int x2;
    int y2;
    double r;
    double g;
    double b;
    double a;
    Persistent<Function> cb;
    gboolean isOk;
};

namespace node {

    Handle<Value> Pixbuf::New(const Arguments &args) {
        HandleScope scope;
        Pixbuf *pb;

        /* new Pixbuf(pixels: Buffer, has_alpha: bool, width: Integer, height: Integer) */
        if (args.Length() == 4 && Buffer::HasInstance(args[0]) && args[1]->IsBoolean() && args[2]->IsInt32() && args[3]->IsInt32()) {
            guchar *pixels = (guchar*) Buffer::Data( args[0]->ToObject() );
            bool has_alpha = args[1]->ToBoolean()->Value();
            int width = args[2]->ToInt32()->Value();
            int height = args[3]->ToInt32()->Value();
            pb = new Pixbuf(pixels, has_alpha, width, height);
        }
        /* new Pixbuf(has_alpha: bool, width: Integer, height: Integer) */
        else if (args.Length() == 3 && args[0]->IsBoolean() && args[1]->IsInt32() && args[2]->IsInt32()) {
            bool has_alpha = args[0]->ToBoolean()->Value();
            int width = args[1]->ToInt32()->Value();
            int height = args[2]->ToInt32()->Value();
            pb = new Pixbuf(has_alpha, width, height);
        }
        /* new Pixbuf(fileName: String, [width: Number, [height: Number, [preserv_aspect_ratio: bool]]]) */
        else if ( 0 < args.Length() && args.Length() < 5 &&
            args[0]->IsString() &&
            ! ( args.Length() > 1 && ! args[1]->IsNumber() ) &&
            ! ( args.Length() > 2 && ! args[2]->IsNumber() ) &&
            ! ( args.Length() > 3 && ! args[3]->IsBoolean() )
        ) {
            GError *err = NULL;
            String::Utf8Value fileName(args[0]);
            GdkPixbuf *tmp;
            if ( args.Length() == 1 ) {
                tmp = gdk_pixbuf_new_from_file(*fileName, &err);
            } else {
                int width = -1, height = -1;
                gboolean preserv_aspect_ratio = true;
                if ( args.Length() > 3 ) preserv_aspect_ratio = args[3]->ToBoolean()->Value();
                if ( args.Length() > 2 ) height = static_cast<int> ( args[2]->ToNumber()->Value() );
                if ( args.Length() > 1 ) width = static_cast<int> ( args[1]->ToNumber()->Value() );
                tmp = gdk_pixbuf_new_from_file_at_scale( *fileName, width, height, preserv_aspect_ratio, &err );
            }
            if (!tmp) {
                Local< String > errStr = String::New(err->message);
                g_clear_error(&err);
                return ThrowException(Exception::Error(errStr));
            }
            pb = new Pixbuf( gdk_pixbuf_get_pixels( tmp ),
                gdk_pixbuf_get_has_alpha( tmp ),
                gdk_pixbuf_get_width( tmp ),
                gdk_pixbuf_get_height( tmp ) );
        } else {
            return ThrowException(Exception::TypeError(String::New("Wrong arguments.")));
        }
        pb->Wrap(args.This());

        return args.This();
    }

    void Pixbuf::Initialize(Handle<Object> target) {
        HandleScope scope;

        red_sym = Persistent<String>::New(String::NewSymbol("r"));
        green_sym = Persistent<String>::New(String::NewSymbol("g"));
        blue_sym = Persistent<String>::New(String::NewSymbol("b"));
        alpha_sym = Persistent<String>::New(String::NewSymbol("a"));

        Local<FunctionTemplate> t = FunctionTemplate::New(Pixbuf::New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("Pixbuf"));

        constructor_template->InstanceTemplate()->SetIndexedPropertyHandler(
            Pixbuf::getPixel, Pixbuf::setPixel, 0, Pixbuf::checkPixel, Pixbuf::enumeratePixel);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("pixels"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("has_alpha"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("width"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("height"), Pixbuf::paramsGetter);
        constructor_template->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), Pixbuf::paramsGetter);

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "toImage", Pixbuf::toImage);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "scale", Pixbuf::scale);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "drawGlyph", Pixbuf::drawGlyph);

        target->Set(String::NewSymbol("Pixbuf"), constructor_template->GetFunction());
    }

    Handle<Value> Pixbuf::paramsGetter(Local< String > property, const AccessorInfo &info) {
        HandleScope scope;

        String::Utf8Value propName(property);
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());

        if (strcmp(*propName, "pixels") == 0) {
            Buffer *pixels = Buffer::New(self->getLength());
            memcpy( Buffer::Data( pixels ), self->getPixels(), self->getLength());
            return scope.Close(pixels->handle_);
        }

        if (strcmp(*propName, "has_alpha") == 0) {
            return scope.Close(Boolean::New(self->hasAlpha()));
        }

        if (strcmp(*propName, "width") == 0) {
            return scope.Close(Uint32::New(self->getWidth()));
        }

        if (strcmp(*propName, "height") == 0) {
            return scope.Close(Uint32::New(self->getHeight()));
        }

        if (strcmp(*propName, "length") == 0) {
            return scope.Close(Uint32::New(self->getWidth() * self->getHeight()));
        }

        return Undefined();
    }

    Handle<Value> Pixbuf::toImage(const Arguments &args) {
        HandleScope scope;
        Pixbuf *src = ObjectWrap::Unwrap<Pixbuf>(args.This());

        if (args.Length() < 1 || args.Length() > 3) {
            return ThrowException(
                Exception::Error(String::New("Wrong arguments: (type: String[, params: Object[, cb: Function]]).")));
        }
        if (!args[0]->IsString()) {
            return ThrowException(
                Exception::Error(String::New("`type` must be a String.")));
        }
        String::Utf8Value type(args[0]);

        struct render_work_t *work = new render_work_t();
        work->request.data = work;
        work->src = src;
        work->type = g_strdup(*type);
        if (args.Length() > 1 && !args[1]->IsFunction() && args[1]->IsObject()) {
            Pixbuf::parseRenderOptions(args[1], &(work->keys), &(work->values), &(work->optc));
        } else {
            work->keys = NULL;
            work->values = NULL;
            work->optc = 0;
        }
        if (args[args.Length() - 1]->IsFunction()) {
            work->cb = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length() - 1]->ToObject()));
            uv_queue_work(uv_default_loop(), &work->request, Pixbuf::render, Pixbuf::afterRender);
            return scope.Close(args.This());
        } else {
            Buffer *buffer;
            Local<Value> error;
            Pixbuf::render(&(work->request));
            if (work->isOk) {
                buffer = Buffer::New(work->buffer_size);
                memcpy(Buffer::Data(buffer), work->buffer, work->buffer_size);
                g_free(work->buffer);
            } else {
                error = Exception::Error(String::New(work->err->message));
                g_error_free(work->err);
            }
            g_free((void *) work->type);
            if (work->keys) g_strfreev(work->keys);
            if (work->values) g_strfreev(work->values);
            delete work;
            return scope.Close(buffer->handle_);
        }
    }

    void Pixbuf::render(uv_work_t* work_req) {
        render_work_t *work = static_cast<render_work_t *>(work_req->data);
        work->isOk = gdk_pixbuf_save_to_bufferv(work->src->getPixbuf(), &(work->buffer), &(work->buffer_size), work->type, work->keys, work->values, &(work->err));
    }

    void Pixbuf::afterRender(uv_work_t* work_req) {
        HandleScope scope;
        render_work_t *work = static_cast<render_work_t *>(work_req->data);
        if (work->isOk) {
            Buffer *buffer = Buffer::New(work->buffer_size);
            memcpy( Buffer::Data( buffer ), work->buffer, work->buffer_size);
            g_free(work->buffer);
            Local<Value> argv[2] = {Local<Value>::New(Null()), Local<Value>::New(buffer->handle_)};
            work->cb->Call(Context::GetCurrent()->Global(), 2, argv);
        } else {
            Local<Value> argv[1] = {Exception::Error(String::New(work->err->message))};
            g_error_free(work->err);
            work->cb->Call(Context::GetCurrent()->Global(), 2, argv);
        }
        g_free((void *)work->type);
        if (work->keys) g_strfreev(work->keys);
        if (work->values) g_strfreev(work->values);
        delete work;
    }

    void Pixbuf::parseRenderOptions(Local<Value> options, gchar ***keys, gchar ***values, uint32_t *optc) {
        HandleScope scope;
        (*keys) = (*values) = NULL;
        *optc = 0;
        if (!options.IsEmpty() && !options->IsNull() && !options->IsUndefined()) {
            Local<Array> v8keys = options->ToObject()->GetPropertyNames();
            uint32_t len = v8keys->Length();
            if (len != 0) {
                for (int i = 0; i < len; i++) {
                    Local<String> v8key = Local<String>::Cast(v8keys->Get(i));
                    String::Utf8Value keystr(v8key);
                    Local<Value> v8value = options->ToObject()->Get(v8key);
                    if (v8value->IsString()) {
                        String::Utf8Value valuestr(v8value);
                        (*optc) = (*optc) + 1;
                        (*keys) = (gchar **)realloc((*keys), sizeof(gchar *) * (*optc)+1);
                        (*values) = (gchar **)realloc((*values), sizeof(gchar *) * (*optc)+1);
                        (*keys)[(*optc)-1] = g_strdup(*keystr);
                        (*values)[(*optc)-1] = g_strdup(*valuestr);
                        (*keys)[(*optc)] = (*values)[(*optc)] = NULL;
                    }
                }
            }
        }
    }

    Handle<Value> Pixbuf::scale(const Arguments &args) {
        HandleScope scope;
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(args.This());
        int width, height;
        GdkPixbuf *newpb;

        if (args.Length() < 2 || !args[0]->IsInt32() || !args[1]->IsInt32())
            return ThrowException(Exception::Error(String::New("Wrong arguments: (width: Int32, height: Int32).")));
        
        width = args[0]->ToInt32()->Value();
        height = args[1]->ToInt32()->Value();

        newpb = gdk_pixbuf_scale_simple(self->getPixbuf(), width, height, GDK_INTERP_BILINEAR);

        if (!newpb)
            return ThrowException(Exception::Error(String::New("Not enough memory to allocate new pixbuf.")));
        
        self->setPixbuf(newpb);

        return scope.Close(args.This());
    }

    Handle<Array> Pixbuf::enumeratePixel(const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;
        int len = self->getWidth() * self->getHeight();

        Local<Array> a = Array::New(len);
        for (int i = 0; i < len; i++) {
            a->Set(i, Uint32::New(i));
        }
        return scope.Close(a);
    }

    Handle<Boolean> Pixbuf::checkPixel(uint32_t index, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;

        return scope.Close(Boolean::New(index < self->getWidth() * self->getHeight() - 1));
    }

    Handle<Value> Pixbuf::getPixel(uint32_t index, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;

        if (index > self->getWidth() * self->getHeight() - 1)
            return Handle<Value>();

        Local<v8::Object> p = v8::Object::New();

        int y = index / self->getHeight();
        int x = index % self->getWidth();

        guchar* pix = self->getPixels() + y * self->getRowstride() + x * self->getNChannels();

        p->Set(red_sym, Uint32::New(pix[0]));
        p->Set(green_sym, Uint32::New(pix[1]));
        p->Set(blue_sym, Uint32::New(pix[2]));;
        if (self->hasAlpha()) {
            p->Set(alpha_sym, Uint32::New(pix[3]));
        }
        return scope.Close(p);
    }

    Handle<Value> Pixbuf::setPixel(uint32_t index, Local<Value> value, const AccessorInfo &info) {
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(info.This());
        HandleScope scope;
        guchar* pix;
        Local<Object> p;

        if (index > self->getWidth() * self->getHeight() - 1)
            return Handle<Value>();
        if (!value->IsObject()) {
            goto error;
        }
        p = value->ToObject();
        goto ok;

        error:

        if (self->hasAlpha())
            return ThrowException(Exception::TypeError(String::New("Pixel must be an object: {r: Integer, g: Integer, b: Integer, a: Integer}")));
        else
            return ThrowException(Exception::TypeError(String::New("Pixel must be an object: {r: Integer, g: Integer, b: Integer}")));

        ok:

        int y = index / self->getHeight();
        int x = index % self->getWidth();

        pix = self->getPixels() + y * self->getRowstride() + x * self->getNChannels();
        if (self->hasAlpha()) {
            if (p->Has(alpha_sym) && p->Get(alpha_sym)->IsUint32()) pix[3] = p->Get(alpha_sym)->ToUint32()->Value();
        }
        if (p->Has(red_sym) && p->Get(red_sym)->IsUint32()) pix[0] = p->Get(red_sym)->ToUint32()->Value();
        if (p->Has(green_sym) && p->Get(green_sym)->IsUint32()) pix[1] = p->Get(green_sym)->ToUint32()->Value();
        if (p->Has(blue_sym) && p->Get(blue_sym)->IsUint32()) pix[2] = p->Get(blue_sym)->ToUint32()->Value();

        return scope.Close(info.This()->Get(index));
    }

    Handle<Value> Pixbuf::drawGlyph(const Arguments &args) {
        HandleScope scope;
        Pixbuf *self = ObjectWrap::Unwrap<Pixbuf>(args.This());
        struct draw_glyph_work_t *work;

        //if (args.Length() < 4 || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() || !args[3]->IsInt32()) {
        //    return ThrowException(Exception::Error(String::New("Wrong arguments: (x1: Int32, y1: Int32, x2: Int32, y2: Int32).")));
        //}

        if (args.Length() < 2 || args.Length() > 3 || !args[0]->IsObject() || !args[1]->IsObject() || (args.Length() == 3 && !args[2]->IsFunction())) {
            return ThrowException(Exception::Error(String::New("Wrong arguments: ({x1,y1,x2,y2},{r,g,b,a}[,cb]).")));
        }

        work = new draw_glyph_work_t();
        work->request.data = work;
        work->src = self;
        work->error = NULL;

        Local<Object> coords = args[0]->ToObject();
        Local<Value> x1 = coords->Get(String::New("x1"));
        Local<Value> x2 = coords->Get(String::New("x2"));
        Local<Value> y1 = coords->Get(String::New("y1"));
        Local<Value> y2 = coords->Get(String::New("y2"));
        if (!x1->IsInt32() || !x2->IsInt32() || !y1->IsInt32() || !y2->IsInt32()) {
            return ThrowException(Exception::Error(String::New("Bad rectangle definition: {'x1': Int32, 'y1': Int32, 'x2': Int32, 'y2': Int32}.")));
        }
        work->x1 = x1->ToInt32()->Value();
        work->x2 = x2->ToInt32()->Value();
        work->y1 = y1->ToInt32()->Value();
        work->y2 = y2->ToInt32()->Value();

        Local<Object> color = args[1]->ToObject();
        Local<Value> r = color->Get(String::New("r"));
        Local<Value> g = color->Get(String::New("g"));
        Local<Value> b = color->Get(String::New("b"));
        Local<Value> a = color->Get(String::New("a"));
        if (!r->IsNumber() || !g->IsNumber() || !b->IsNumber() || !a->IsNumber()) {
            return ThrowException(Exception::Error(String::New("Bad color definition: {'r': Number, 'g': Number, 'b': Number, 'a': Number}.")));
        }
        work->r = r->ToNumber()->Value();
        work->g = g->ToNumber()->Value();
        work->b = b->ToNumber()->Value();
        work->a = a->ToNumber()->Value();

        if (args[args.Length() - 1]->IsFunction()) {
            work->cb = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length() - 1]->ToObject()));
            uv_queue_work(uv_default_loop(), &work->request, Pixbuf::_drawGlyph, Pixbuf::afterDrawGlyph);
        } else {
            Local<Value> error;
            Pixbuf::_drawGlyph(&(work->request));
            if (!work->isOk) {
                error = Exception::Error(String::New(work->error));
                g_free((void *) work->error);
                delete work;
                return ThrowException(error);
            }
            delete work;
        }
        return scope.Close(args.This());
    }

    void Pixbuf::_drawGlyph(uv_work_t* work_req) {
        draw_glyph_work_t *work = static_cast<draw_glyph_work_t *>(work_req->data);
        // work->isOk = gdk_pixbuf_save_to_bufferv(work->src->getPixbuf(), &(work->buffer), &(work->buffer_size), work->type, work->keys, work->values, &(work->err));
        int width, height, rowstride, n_channels, x1, y1, x2, y2;
        guchar *pixels, *p;
        double r, g, b, a, nr, ng, nb;

        r = work->r;
        g = work->g;
        b = work->b;
        a = work->a;

        x1 = work->x1;
        x2 = work->x2;
        y1 = work->y1;
        y2 = work->y2;

        n_channels = work->src->getNChannels();
        width = work->src->getWidth();
        height = work->src->getHeight();
        rowstride = work->src->getRowstride();
        pixels = work->src->getPixels();

        if (x1 < 0 || x2 < x1 || y1 < 0 || y2 < y1 || x2 >= width || y2 >= height) {
            work->error = g_strdup("Glyph out of image geometry");
            work->isOk = false;
        } else if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255) {
            work->error = g_strdup("Color component value must be in range 0..255 inclusively");
            work->isOk = false;
        } else {
            for (int ty1 = y1, ty2 = y2;ty1 <= ty2; ty1++) {
                for (int tx1 = x1, tx2 = x2;tx1 <= tx2; tx1++) {
                    p = pixels + ty1 * rowstride + tx1 * n_channels;
                    nr = (0xff - a) * ((float)p[0] / 0xff) + a * (r / 0xff);
                    ng = (0xff - a) * ((float)p[1] / 0xff) + a * (g / 0xff);
                    nb = (0xff - a) * ((float)p[2] / 0xff) + a * (b / 0xff);
                    p[0] = (nr > 255) ? 255 : nr;
                    p[1] = (ng > 255) ? 255 : ng;
                    p[2] = (nb > 255) ? 255 : nb;
                }
            }
            work->isOk = true;
        }
    }

    void Pixbuf::afterDrawGlyph(uv_work_t* work_req) {
        HandleScope scope;
        draw_glyph_work_t *work = static_cast<draw_glyph_work_t *>(work_req->data);
        if (work->isOk) {
            Local<Value> argv[1] = {Local<Value>::New(Null())};
            work->cb->Call(Context::GetCurrent()->Global(), 1, argv);
        } else {
            if (work->error) {
                Local<Value> argv[1] = {Exception::Error(String::New(work->error))};
                g_free((void*) work->error);
                work->cb->Call(Context::GetCurrent()->Global(), 1, argv);
            } else {
                Local<Value> argv[1] = {Exception::Error(String::New("Unexpected error"))};
                work->cb->Call(Context::GetCurrent()->Global(), 1, argv);
            }
        }
    }
}

// Exporting function
extern "C" void
init (v8::Handle<v8::Object> target)
{
    HandleScope scope;

    //Required for gdk
    g_type_init();

    Pixbuf::Initialize(target);
}
