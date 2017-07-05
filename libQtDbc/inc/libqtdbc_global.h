#ifndef QLIBDBC_GLOBAL_H
#define QLIBDBC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QLIBDBC_LIBRARY)
#  define QLIBDBCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QLIBDBCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QLIBDBC_GLOBAL_H
