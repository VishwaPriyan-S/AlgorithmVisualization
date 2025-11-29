#include "PythonParserEngine.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QDir>
#include <QCoreApplication>
#include <QString>
#include <QByteArray>

PythonParserEngine::PythonParserEngine(const std::string &pythonExecutable)
    : m_pythonExe(pythonExecutable)
{
}

std::string PythonParserEngine::parseToAstJson(const std::string &code)
{
    // 1. Write user code to a temporary .py file
    QTemporaryFile tmpFile(QDir::tempPath() + "/algovisXXXXXX.py");
    if (!tmpFile.open()) {
        return "{\"error\":\"Failed to create temporary Python file\"}";
    }

    tmpFile.write(code.data(), static_cast<qint64>(code.size()));
    tmpFile.flush();

    QString codeFilePath = tmpFile.fileName();

    // 2. Build path to our Python helper script
    //    Expecting: <app_dir>/python/python_parser_engine.py
    QString scriptPath = QCoreApplication::applicationDirPath()
                         + "/python/python_parser_engine.py";

    if (!QFile::exists(scriptPath)) {
        std::string err = std::string("{\"error\":\"python_parser_engine.py not found at: ")
                          + scriptPath.toStdString() + "\"}";
        return err;
    }

    // 3. Run: python python_parser_engine.py <temp_file>
    QProcess proc;
    QString program = QString::fromStdString(m_pythonExe);
    QStringList args;
    args << scriptPath << codeFilePath;

    proc.start(program, args);
    if (!proc.waitForStarted(5000)) {
        return "{\"error\":\"Failed to start Python process\"}";
    }

    proc.waitForFinished(-1);

    QByteArray stdoutData = proc.readAllStandardOutput();
    QByteArray stderrData = proc.readAllStandardError();

    if (proc.exitStatus() != QProcess::NormalExit) {
        return "{\"error\":\"Python process crashed\"}";
    }

    if (proc.exitCode() != 0) {
        // Python script signaled error; include stderr if any
        std::string err = "{\"error\":\"Python parser error\",\"stderr\":\""
                          + std::string(stderrData.constData()).substr(0, 200)
                          + "\"}";
        return err;
    }

    if (stdoutData.isEmpty()) {
        return "{\"error\":\"Python parser produced no output\"}";
    }

    return stdoutData.toStdString();
}
