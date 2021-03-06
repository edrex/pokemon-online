#include "sql.h"
#include "sqlconfig.h"
#include "../Utilities/otherwidgets.h"
#include "security.h"
#include "tiermachine.h"

SQLConfigWindow::SQLConfigWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *v = new QVBoxLayout(this);

    QLabel *desc = new QLabel(tr("<b><span style='color:red'>Don't touch anything if you've no clue what SQL is!</span></b><br /><br />For any change to have effect, you need to restart the server."
                                 "<br />If you change the settings without knowledge of what you are doing, you'll probably end up without any users stored anymore.<br/><br/>SQLite is the "
                                 "only system fully supported by default. PostGreSQL needs an external installation, and you then just have to put the .dlls in that are located in PostGreSQL's bin folder in the server folder. "
                                 "MySQL needs the user to get the right DLLs, the MySQL driver and to install a MySQL database too (it is advised to be on linux to do this as this is far less complicated)."));
    desc->setWordWrap(true);
    v->addWidget(desc);

    QSettings s;

    b = new QComboBox();
    b->addItem("SQLite");
    b->addItem("PostGreSQL");
    b->addItem("MySQL");
    v->addLayout(new QSideBySide(new QLabel(tr("SQL Database type: ")), b));
    if (s.value("sql_driver").toInt() == SQLCreator::PostGreSQL) {
        b->setCurrentIndex(1);
    } else if (s.value("sql_driver").toInt() == SQLCreator::MySQL) {
        b->setCurrentIndex(2);
    }

    name = new QLineEdit();
    name->setText(s.value("sql_db_name").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Database name: ")), name));

    user = new QLineEdit();
    user->setText(s.value("sql_db_user").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("User: ")), user));

    pass = new QLineEdit();
    pass->setText(s.value("sql_db_pass").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Password: ")), pass));

    host = new QLineEdit();
    host->setText(s.value("sql_db_host").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Host: ")), host));

    port = new QSpinBox();
    port->setRange(0, 65535);
    port->setValue(s.value("sql_db_port").toInt());
    v->addLayout(new QSideBySide(new QLabel(tr("Port: ")), port));

    QPushButton *exporting = new QPushButton(tr("&Export"));
    connect(exporting, SIGNAL(clicked()), SLOT(exportDatabase()));

    QPushButton *apply = new QPushButton(tr("&Apply"));
    connect(apply, SIGNAL(clicked()), this, SLOT(apply()));

    v->addLayout(new QSideBySide(exporting, apply));

    connect(b, SIGNAL(activated(int)), SLOT(changeEnabled()));
    changeEnabled();
}

void SQLConfigWindow::changeEnabled()
{
    bool c = (b->currentIndex() == 0);

    user->setDisabled(c);
    port->setDisabled(c);
    host->setDisabled(c);
    pass->setDisabled(c);
}

void SQLConfigWindow::apply()
{
    QSettings s;

    s.setValue("sql_driver", b->currentIndex());
    s.setValue("sql_db_name", name->text());
    s.setValue("sql_db_port", port->value());
    s.setValue("sql_db_user", user->text());
    s.setValue("sql_db_pass", pass->text());
    s.setValue("sql_db_host", host->text());
}

void SQLConfigWindow::exportDatabase()
{
    if (QMessageBox::question(this, "Exporting all the data", "Exporting will create a backup of the database with .txt files, and may hang the server a little."
                              " Are you sure you want to continue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    SecurityManager::exportDatabase();
    TierMachine::obj()->exportDatabase();

    QMessageBox::information(this, "Database Saved!", "The database was saved successfully (members.txt and tier_*.txt files). If you want to import it"
                             " from another server, put those files in the other server's directory and make sure the database it has access to is empty,"
                             " otherwise it won't import. You also have to restart the other server.");
}
