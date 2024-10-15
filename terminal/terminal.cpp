/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"

Terminal::Terminal(Kernel *core, QWidget *parent) : QWidget{parent}
{
    kernel = core;
    grid = new QVBoxLayout(this);
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    connect(scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(scrollToLastMessage(int, int)));
    messages = new QVBoxLayout();
    QFrame *messagesFrame = new QFrame();
    messagesFrame->setLayout(messages);
    scrollArea->setWidget(messagesFrame);
    messages->addStretch();
    grid->addWidget(scrollArea);
    prompt = new QLabel();
    prompt->setWordWrap(true);
    grid->addWidget(prompt);
}

void Terminal::write(int key)
{
    command.append(key);
    prompt->setText(promptText(command));
    QMutexLocker locker(kernel->mutex);
    updateInspector(key);
}

void Terminal::backspace()
{
    if (command.isEmpty()) {
        return;
    }
    command.removeLast();
    prompt->setText(promptText(command));
    QMutexLocker locker(kernel->mutex);
    for (int key : command) {
        updateInspector(key);
    }
}

void Terminal::clear() {
    command.clear();
    prompt->setText(promptText(command));
}

void Terminal::execute(bool clear)
{
    if(command.isEmpty()) {
        return;
    }
    int selectionType = command[0];
    QString response = kernel->execute(command);
    info(">>> " + promptText(command));
    if (!response.isEmpty()) {
        error(response);
    } else {
        if (clear) {
            this->clear();
        }
    }
    QMutexLocker locker(kernel->mutex);
    updateInspector(selectionType);
}

bool Terminal::execute(QString command) {
    QList<int> commandKeys;
    QString text = QString();
    for (qsizetype index = 0; index < command.size(); index++) {
        if (command.at(index) == QChar('+')) {
            commandKeys.append(keys::plus);
        } else if (command.at(index) == QChar('-')) {
            commandKeys.append(keys::minus);
        } else if (command.at(index) == QChar('.')) {
            commandKeys.append(keys::period);
        } else if (command.at(index) == QChar(',')) {
            commandKeys.append(keys::comma);
        } else if (command.at(index) == QChar('*')) {
            commandKeys.append(keys::thru);
        } else if (command.at(index) == QChar('0')) {
            commandKeys.append(keys::zero);
        } else if (command.at(index) == QChar('1')) {
            commandKeys.append(keys::one);
        } else if (command.at(index) == QChar('2')) {
            commandKeys.append(keys::two);
        } else if (command.at(index) == QChar('3')) {
            commandKeys.append(keys::three);
        } else if (command.at(index) == QChar('4')) {
            commandKeys.append(keys::four);
        } else if (command.at(index) == QChar('5')) {
            commandKeys.append(keys::five);
        } else if (command.at(index) == QChar('6')) {
            commandKeys.append(keys::six);
        } else if (command.at(index) == QChar('7')) {
            commandKeys.append(keys::seven);
        } else if (command.at(index) == QChar('8')) {
            commandKeys.append(keys::eight);
        } else if (command.at(index) == QChar('9')) {
            commandKeys.append(keys::nine);
        } else if (command.at(index) == QChar('m')) {
            commandKeys.append(keys::model);
        } else if (command.at(index) == QChar('f')) {
            commandKeys.append(keys::fixture);
        } else if (command.at(index) == QChar('g')) {
            commandKeys.append(keys::group);
        } else if (command.at(index) == QChar('i')) {
            commandKeys.append(keys::intensity);;
        } else if (command.at(index) == QChar('c')) {
            commandKeys.append(keys::color);
        } else if (command.at(index) == QChar('t')) {
            commandKeys.append(keys::transition);
        } else if (command.at(index) == QChar('r')) {
            commandKeys.append(keys::row);
        } else if (command.at(index) == QChar('q')) {
            commandKeys.append(keys::cue);
        } else if (command.at(index) == QChar('C')) {
            commandKeys.append(keys::copyItem);
        } else if (command.at(index) == QChar('D')) {
            commandKeys.append(keys::deleteItem);
        } else if (command.at(index) == QChar('L')) {
            commandKeys.append(keys::labelItem);
        } else if (command.at(index) == QChar('M')) {
            commandKeys.append(keys::moveItem);
        } else if (command.at(index) == QChar('R')) {
            commandKeys.append(keys::recordItem);
        } else if (command.at(index) == QChar('"')) {
            text = command.mid((index + 1), (command.length() - index - 2));
        } else {
            return false;
        }
        if (!text.isEmpty()) {
            break;
        }
    }
    QString response = kernel->execute(commandKeys, text);
    info("file> " + promptText(commandKeys));
    if (!response.isEmpty()) {
        error(response);
        return false;
    }
    return true;
}

void Terminal::info(QString message)
{
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    messages->addWidget(label);
    qInfo() << message;
}

void Terminal::success(QString message)
{
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    label->setStyleSheet("color: green;");
    messages->addWidget(label);
    qInfo() << message;
}

void Terminal::warning(QString message)
{
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    label->setStyleSheet("color: yellow;");
    messages->addWidget(label);
    qWarning() << message;
}

void Terminal::error(QString message)
{
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    label->setStyleSheet("color: red;");
    messages->addWidget(label);
    qWarning() << message;
}

QString Terminal::promptText(QList<int> keys)
{
    QString commandString = "";
    for(const int key: keys) {
        if (key == keys::plus) {
            commandString += " + ";
        } else if (key == keys::minus) {
            commandString += " - ";
        } else if (key == keys::period) {
            commandString += ".";
        } else if (key == keys::comma) {
            commandString += ",";
        } else if (key == keys::thru) {
            commandString += " Thru ";
        } else if (key == keys::zero) {
            commandString += "0";
        } else if (key == keys::one) {
            commandString += "1";
        } else if (key == keys::two) {
            commandString += "2";
        } else if (key == keys::three) {
            commandString += "3";
        } else if (key == keys::four) {
            commandString += "4";
        } else if (key == keys::five) {
            commandString += "5";
        } else if (key == keys::six) {
            commandString += "6";
        } else if (key == keys::seven) {
            commandString += "7";
        } else if (key == keys::eight) {
            commandString += "8";
        } else if (key == keys::nine) {
            commandString += "9";
        } else if (key == keys::model) {
            commandString += " Model ";
        } else if (key == keys::fixture) {
            commandString += " Fixture ";
        } else if (key == keys::group) {
            commandString += " Group ";
        } else if (key == keys::intensity) {
            commandString += " Intensity ";
        } else if (key == keys::color) {
            commandString += " Color ";
        } else if (key == keys::transition) {
            commandString += " Transition ";
        } else if (key == keys::row) {
            commandString += " Row ";
        } else if (key == keys::cue) {
            commandString += " Cue ";
        } else if (key == keys::copyItem) {
            commandString += " Copy ";
        } else if (key == keys::deleteItem) {
            commandString += " Delete ";
        } else if (key == keys::labelItem) {
            commandString += " Label ";
        } else if (key == keys::moveItem) {
            commandString += " Move ";
        } else if (key == keys::recordItem) {
            commandString += " Record ";
        } else {
            error(tr("Unknown key pressed: %1").arg(key));
        }
    }
    commandString.replace("  ", " "); // Remove double whitespaces
    if (commandString.startsWith(" ")) { // if first character is a whitespace
        commandString.remove(0, 1); // remove first character
    }
    if (commandString.endsWith(" ")) { // if last character is a whitespace
        commandString.remove(commandString.size(), 1); // remove last character
    }
    return commandString;
}

void Terminal::updateInspector(int key) {
    if (key == keys::model) {
        kernel->inspector->loadModels();
    } else if (key == keys::fixture) {
        kernel->inspector->loadFixtures();
    } else if (key == keys::group) {
        kernel->inspector->loadGroups();
    } else if (key == keys::intensity) {
        kernel->inspector->loadIntensities();
    } else if (key == keys::color) {
        kernel->inspector->loadColors();
    } else if (key == keys::transition) {
        kernel->inspector->loadTransitions();
    } else if (key == keys::row) {
        kernel->inspector->loadRows();
    } else if (key == keys::cue) {
        kernel->inspector->loadCues();
    }
}

void Terminal::scrollToLastMessage(int min, int max)
{
    Q_UNUSED(min);
    scrollArea->verticalScrollBar()->setValue(max);
}
