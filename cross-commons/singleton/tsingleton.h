#ifndef TSINGLETON_H
#define TSINGLETON_H

//! шаблон для singleton. T - создаваемый объект. C - параметр для конструктора
struct NIL {};
template<class T, typename C=NIL> class TSingleton {
public:
  static T* instance(C p) {
    if( 0 == _instance ) {
      _instance = new T(p);
    }
    return _instance;
  }

  static T* instance() {
    if( 0 == _instance ) {
      _instance = new T;
    }
    return _instance;
  }


  static void free() {
    if (_instance) delete _instance;
    _instance = 0;
  }

private:
  TSingleton();
  TSingleton(const TSingleton&);
  TSingleton& operator=(const TSingleton&);
  virtual ~TSingleton();
  
private:
  static T* _instance;
};

#endif
