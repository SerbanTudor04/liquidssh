#include "TerminalView.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QTextOption>
#include <QTextCursor>

TerminalView::TerminalView(QWidget *parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setFocusPolicy(Qt::StrongFocus);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    f.setPointSizeF(12);
    setFont(f);

    setWordWrapMode(QTextOption::NoWrap);
    setBackgroundVisible(false);
    setFrameStyle(QFrame::NoFrame);
    document()->setMaximumBlockCount(5000);

}

void TerminalView::appendRemote(const QByteArray& data) {
    QByteArray clean = ansiStrip(data);
    auto c = textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(QString::fromUtf8(clean));
    setTextCursor(c);
    ensureCursorVisible();
}

void TerminalView::clearScreen(){
    document()->clear();
    auto c  = textCursor();
    c.movePosition(QTextCursor::StartOfBlock);
    setTextCursor(c);
}

QByteArray TerminalView::ansiStrip(const QByteArray& in) const {
    static QRegularExpression re("\x1B\\[[0-9;?]*[ -/]*[@-~]");
    QString t = QString::fromUtf8(in);
    t.remove(re);
    return t.toUtf8();
}

void TerminalView::keyPressEvent(QKeyEvent *e) {
    if (e->matches(QKeySequence::Paste)) {
        emit sendBytes(QGuiApplication::clipboard()->text().toUtf8());
        return;
    }
    QByteArray out;
    switch (e->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter: out = "\r"; break;
        case Qt::Key_Backspace: out = "\x7f"; break;
        case Qt::Key_Tab: out = "\t"; break;
        case Qt::Key_Left:  out = "\x1b[D"; break;
        case Qt::Key_Right: out = "\x1b[C"; break;
        case Qt::Key_Up:    out = "\x1b[A"; break;
        case Qt::Key_Down:  out = "\x1b[B"; break;
        case Qt::Key_Home:  out = "\x1b[H"; break;
        case Qt::Key_End:   out = "\x1b[F"; break;
        case Qt::Key_PageUp:   out = "\x1b[5~"; break;
        case Qt::Key_PageDown: out = "\x1b[6~"; break;
        default:
            if (!e->text().isEmpty()) out = e->text().toUtf8();
            break;
    }
    if (!out.isEmpty()) emit sendBytes(out);
}
