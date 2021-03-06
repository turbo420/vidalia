/*
**  This file is part of Vidalia, and is subject to the license terms in the
**  LICENSE file, found in the top level directory of this distribution. If you
**  did not receive the LICENSE file with this file, you may obtain it from the
**  Vidalia source package distributed by the Vidalia Project at
**  http://www.vidalia-project.net/. No part of Vidalia, including this file,
**  may be copied, modified, propagated, or distributed except according to the
**  terms described in the LICENSE file.
*/

/*
** \file Log.cpp
** \brief Debug message logging
*/

#include "Log.h"

#include <QDateTime>
#include <QTextStream>

/** Open log files for appending as write-only text. */
#define LOGFILE_MODE  \
  (QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)
/** Format for log message timestamps. */
#define TIMESTAMP_FMT   "MMM dd HH:mm:ss.zzz"


/** Default constructor. Logs at level Notice by default. */
Log::Log()
{
  _logLevel = Notice;
}

/** Destructor. Closes the log file. */
Log::~Log()
{
  close();
}

/** Returns a list of strings representing available log levels. */
QStringList
Log::logLevels()
{
  return (QStringList() << "debug" << "info" << "notice" 
                        << "warn" << "error");
}

/** Sets the current log level to <b>level</b>. If <b>level</b> is Off, then
 * the log file will be closed as well. If <b>level</b> is Unknown, no change
 * to the current log level is made. */
void
Log::setLogLevel(LogLevel level)
{
  if (level >= Debug && level < Unknown)
    _logLevel = level;
  if (level == Off)
    _logFile.close();
}

/** Opens <b>file</b> for appending, to which log messages will be written. */
bool
Log::open(FILE *file)
{
  if (_logFile.isOpen())
    close();

  _logFile.open(file, LOGFILE_MODE);
  return isOpen();
}

/** Opens <b>file</b> for appending, to which log messages will be written. */
bool
Log::open(QString file)
{
  if (_logFile.isOpen())
    close();

  _logFile.setFileName(file);
  _logFile.open(LOGFILE_MODE);
  return isOpen();
}

/** Flushes any outstanding log messages and closes the log file. */
void
Log::close()
{
  if (_logFile.isOpen()) {
    _logFile.flush();
    _logFile.close();
  }
}

/** Creates a log message with severity <b>level</b> and initial message
 * contents <b>message</b>. The log message can be appended to until the
 * returned LogMessage's destructor is called, at which point the complete
 * message is written to the log file. */
inline Log::LogMessage
Log::log(LogLevel level)
{
  if (level < _logLevel)
    return LogMessage(level, 0);
  return LogMessage(level, &_logFile);
}

/** Creates a log message with severity <b>level</b>. The log message can be
 * appended to until the returned LogMessage's destructor is called, at
 * which point the complete message is written to the log file. */
Log::LogMessage
Log::log(LogLevel level, QString msg)
{
  return log(level) << msg;
}

/** Returns a string description of the given LogLevel <b>level</b>. */
inline QString
Log::logLevelToString(LogLevel level)
{
  switch (level) {
    case Debug:   return "debug";
    case Info:    return "info";
    case Notice:  return "notice";
    case Warn:    return "warn";
    case Error:   return "error";
    case Off:     return "off";
    default:      return "unknown";
  }
}

/** Returns a LogLevel for the level given by <b>str</b>, or Unknown if the
 * given string does not represent a valid LogLevel value. */
Log::LogLevel
Log::stringToLogLevel(QString str)
{
  str = str.toLower();
  if (str == "debug")
    return Debug;
  else if (str == "info")
    return Info;
  else if (str == "notice")
    return Notice;
  else if (str == "warn")
    return Warn;
  else if (str == "error")
    return Error;
  else if (str == "off")
    return Off;
  return Unknown;
}

/** Returns a formatted log message, prefixed with a timestamp and the log
 * message severity level. */
inline QString
Log::LogMessage::toString() const
{
  QString msg = QDateTime::currentDateTime().toString(TIMESTAMP_FMT);
  msg.append(" [" + Log::logLevelToString(stream->type) + "] ");
  msg.append(stream->buf);
  return msg;
}

/** Destructor. Writes the buffered log message out to the log file specified
 * in the constructor. */
Log::LogMessage::~LogMessage()
{
  if (!--stream->ref) {
    if (stream->out && !stream->buf.isEmpty()) {
      QTextStream log(stream->out);
      log << toString() << endl;
      log.flush();
    }
    delete stream;
  }
}

