#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <curl/curl.h>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

using namespace std;

QProcess* executeCommand(const QString& program, const QStringList& arguments, const QString& workingDirectory);
