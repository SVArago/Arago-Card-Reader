#ifndef FRONTEND_H_
#define FRONTEND_H_

#include <QString>

// NOTE: Implementations of these functions must be thread-safe
void frontend_message(QString);
void frontend_error(QString);

#endif