#include "runner.h"

QProcess* executeCommand(const QString& prg, const QStringList& arg, const QString& wDir) {
    QProcess* process = new QProcess();

    process->setWorkingDirectory(wDir);

    QObject::connect(process, &QProcess::readyReadStandardOutput, process, [process]() {
        qDebug().noquote() << process->readAllStandardOutput();
    });

    process->start(prg, arg);

    return process;
}