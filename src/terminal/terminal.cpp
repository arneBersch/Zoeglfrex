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
    kernel->inspector->loadItemList(key);
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
        kernel->inspector->loadItemList(key);
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
    info(">>> " + promptText(command));
    kernel->execute(command);
    if (clear) {
        this->clear();
    }
    QMutexLocker locker(kernel->mutex);
    kernel->inspector->loadItemList(selectionType);
}

void Terminal::execute(QString command, QString action) {
    QList<int> commandKeys;
    QString text = QString();
    for (qsizetype index = 0; index < command.size(); index++) {
        if (command.at(index) == QChar('+')) {
            commandKeys.append(Keys::Plus);
        } else if (command.at(index) == QChar('-')) {
            commandKeys.append(Keys::Minus);
        } else if (command.at(index) == QChar('.')) {
            commandKeys.append(Keys::Period);
        } else if (command.at(index) == QChar('*')) {
            commandKeys.append(Keys::Asterisk);
        } else if (command.at(index) == QChar('/')) {
            commandKeys.append(Keys::Thru);
        } else if (command.at(index) == QChar('0')) {
            commandKeys.append(Keys::Zero);
        } else if (command.at(index) == QChar('1')) {
            commandKeys.append(Keys::One);
        } else if (command.at(index) == QChar('2')) {
            commandKeys.append(Keys::Two);
        } else if (command.at(index) == QChar('3')) {
            commandKeys.append(Keys::Three);
        } else if (command.at(index) == QChar('4')) {
            commandKeys.append(Keys::Four);
        } else if (command.at(index) == QChar('5')) {
            commandKeys.append(Keys::Five);
        } else if (command.at(index) == QChar('6')) {
            commandKeys.append(Keys::Six);
        } else if (command.at(index) == QChar('7')) {
            commandKeys.append(Keys::Seven);
        } else if (command.at(index) == QChar('8')) {
            commandKeys.append(Keys::Eight);
        } else if (command.at(index) == QChar('9')) {
            commandKeys.append(Keys::Nine);
        } else if (command.at(index) == QChar('m')) {
            commandKeys.append(Keys::Model);
        } else if (command.at(index) == QChar('f')) {
            commandKeys.append(Keys::Fixture);
        } else if (command.at(index) == QChar('g')) {
            commandKeys.append(Keys::Group);
        } else if (command.at(index) == QChar('i')) {
            commandKeys.append(Keys::Intensity);;
        } else if (command.at(index) == QChar('c')) {
            commandKeys.append(Keys::Color);
        } else if (command.at(index) == QChar('q')) {
            commandKeys.append(Keys::Cue);
        } else if (command.at(index) == QChar('C')) {
            commandKeys.append(Keys::Copy);
        } else if (command.at(index) == QChar('D')) {
            commandKeys.append(Keys::Delete);
        } else if (command.at(index) == QChar('L')) {
            commandKeys.append(Keys::Label);
        } else if (command.at(index) == QChar('M')) {
            commandKeys.append(Keys::Move);
        } else if (command.at(index) == QChar('R')) {
            commandKeys.append(Keys::Record);
        } else if (command.at(index) == QChar('"')) {
            text = command.mid((index + 1), (command.length() - index - 2));
        } else {
            return;
        }
        if (!text.isEmpty()) {
            break;
        }
    }
    info(action + "> " + promptText(commandKeys));
    kernel->execute(commandKeys, text);
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
        if (key == Keys::Plus) {
            commandString += " + ";
        } else if (key == Keys::Minus) {
            commandString += " - ";
        } else if (key == Keys::Period) {
            commandString += ".";
        } else if (key == Keys::Asterisk) {
            commandString += "*";
        } else if (key == Keys::Thru) {
            commandString += " Thru ";
        } else if (key == Keys::Zero) {
            commandString += "0";
        } else if (key == Keys::One) {
            commandString += "1";
        } else if (key == Keys::Two) {
            commandString += "2";
        } else if (key == Keys::Three) {
            commandString += "3";
        } else if (key == Keys::Four) {
            commandString += "4";
        } else if (key == Keys::Five) {
            commandString += "5";
        } else if (key == Keys::Six) {
            commandString += "6";
        } else if (key == Keys::Seven) {
            commandString += "7";
        } else if (key == Keys::Eight) {
            commandString += "8";
        } else if (key == Keys::Nine) {
            commandString += "9";
        } else if (key == Keys::Model) {
            commandString += " Model ";
        } else if (key == Keys::Fixture) {
            commandString += " Fixture ";
        } else if (key == Keys::Group) {
            commandString += " Group ";
        } else if (key == Keys::Intensity) {
            commandString += " Intensity ";
        } else if (key == Keys::Color) {
            commandString += " Color ";
        } else if (key == Keys::Cue) {
            commandString += " Cue ";
        } else if (key == Keys::Copy) {
            commandString += " Copy ";
        } else if (key == Keys::Delete) {
            commandString += " Delete ";
        } else if (key == Keys::Label) {
            commandString += " Label ";
        } else if (key == Keys::Move) {
            commandString += " Move ";
        } else if (key == Keys::Record) {
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

void Terminal::scrollToLastMessage(int min, int max)
{
    Q_UNUSED(min);
    scrollArea->verticalScrollBar()->setValue(max);
}
