#pragma once

#include "../algorithm_base.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

#include <QObject>
#include <QThread>
#include <vector>
#include <string>

class InterpreterAlgorithm : public AlgorithmBase
{
    Q_OBJECT

public:

    explicit InterpreterAlgorithm(const QString &user_code,
                                  QObject *parent = nullptr)
        : AlgorithmBase(parent),
        source_code(user_code.toStdString())
    {}

    QString name() const override { return "Interpreter Algorithm"; }
    QString description() const override { return "Executes user-defined pseudo code with visualization"; }
    QString getSourceCode() const override { return QString::fromStdString(source_code); }
    AlgorithmType type() const override { return AlgorithmType::Other; }
    QString getComplexityString() const override { return "User-defined"; }

    void execute(const QVector<int>& data) override
    {
        // Convert QVector<int> → std::vector<int>
        std::vector<int> numericData(data.begin(), data.end());

        // ------------------------
        // 1. LEX + PARSE
        // ------------------------
        Lexer lexer(source_code);
        std::vector<Token> tokens;

        try {
            tokens = lexer.tokenize();
        } catch (const std::exception &ex) {
            emit algorithmError("Lexer Error: " + QString(ex.what()));
            emit algorithmFinished();
            return;
        }

        Parser parser(tokens);

        // IMPORTANT FIX: move construction, not copy
        std::vector<Function> functions = parser.parse();   // ✔ SAFE, uses move

        // ------------------------
        // 2. INTERPRETER
        // ------------------------
        Interpreter interp(functions);

        // ------------------------
        // 3. CALLBACK → convert VisEvent into StepData
        // ------------------------
        interp.setCallback([this](const VisEvent &ev) {

            HighlightInfo highlight;
            VisualizationData viz;

            StepData step(highlight, viz);

            switch (ev.type) {

            case VisEvent::Type::Call:
                step.setStepType(StepData::FUNCTION_CALL);
                step.setDescription("Function Call: " + ev.name);
                break;

            case VisEvent::Type::Return:
                step.setStepType(StepData::FUNCTION_RETURN);
                step.setDescription(
                    "Return: " + ev.name + " = " + QString::number(ev.value)
                    );
                break;

            case VisEvent::Type::Compare:
                step.setStepType(StepData::COMPARISON);
                step.setDescription("Compare");
                break;

            case VisEvent::Type::Assign:
                step.setStepType(StepData::ASSIGNMENT);
                step.setDescription(
                    QString("Assign: index %1 = %2")
                        .arg(ev.i)
                        .arg(ev.value)
                    );
                break;

            default:
                step.setStepType(StepData::LOOP_START);
                step.setDescription("Highlight");
                break;
            }

            m_steps.append(step);
            emit stepChanged(m_steps.size() - 1, m_steps.size());

            QThread::msleep(run_delay_ms);
        });

        // ------------------------
        // 4. CALL FIRST FUNCTION
        // ------------------------
        try {
            std::vector<long long> args;
            if (!numericData.empty())
                args.push_back(numericData[0]);

            if (!functions.empty())
                interp.callFunction(functions[0].name, args);

        } catch (const std::exception &ex) {
            emit algorithmError("Runtime Error: " + QString(ex.what()));
        }

        emit algorithmFinished();
    }

    void setDelay(int ms) { run_delay_ms = ms; }

private:
    std::string source_code;
    int run_delay_ms = 40;
};
