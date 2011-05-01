namespace ruby {

struct VideoQtRasterContext {
  QImage *image;
  unsigned width, height;
  unsigned filter;
} context;

class pVideoQtRaster {
public:
  QWidget *parent;
  QVBoxLayout *layout;

  struct QtImage : public QWidget {
    VideoQtRasterContext &context;

    void paintEvent(QPaintEvent*) {
      if(context.image == 0) return;
      QPainter painter(this);

      if(size().width() == context.width && size().height() == context.height) {
        painter.drawImage(0, 0, *context.image);
      } else {
        Qt::TransformationMode mode = Qt::FastTransformation;
        if(context.filter == Video::FilterLinear) mode = Qt::SmoothTransformation;
        painter.drawImage(0, 0, context.image->scaled(size(), Qt::IgnoreAspectRatio, mode));
      }
    }

    QtImage(QWidget *parent, VideoQtRasterContext &context_) : QWidget(parent), context(context_) {}
  } *widget;

  bool cap(const string& name) {
    if(name == Video::Filter) return true;
    if(name == "QWidget") return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Filter) return context.filter;
    if(name == "QWidget") return parent;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Filter) {
      context.filter = any_cast<unsigned>(value);
      return true;
    }

    if(name == "QWidget") {
      parent = any_cast<QWidget*>(value);
      return true;
    }

    return false;
  }

  void resize(unsigned width, unsigned height) {
    if(context.width != width || context.height != height) {
      if(context.image) delete context.image;
      context.image = new QImage(context.width = width, context.height = height, QImage::Format_RGB32);
    }
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    //if image size has changed since last lock(), re-allocate buffer to match new size
    if(width != context.width || height != context.height) resize(width, height);

    pitch = width * sizeof(uint32_t);
    return data = (uint32_t*)context.image->bits();
  }

  void unlock() {
  }

  void clear() {
    context.image->fill(0);
    widget->update();
  }

  void refresh() {
    widget->update();
  }

  bool init() {
    term();

    layout = new QVBoxLayout;
    layout->setMargin(0);

    context.image  = 0;
    context.width  = 0;
    context.height = 0;
    context.filter = Video::FilterPoint;
    resize(256, 256);

    widget = new QtImage(parent, context);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(widget);
    parent->setLayout(layout);
    clear();

    return true;
  }

  void term() {
    if(context.image) delete context.image;
    if(widget) delete widget;
    if(layout) delete layout;

    context.image = 0;
    widget = 0;
    layout = 0;
  }

  pVideoQtRaster() {
    context.image = 0;
    widget = 0;
    layout = 0;
  }
};

DeclareVideo(QtRaster)

};
