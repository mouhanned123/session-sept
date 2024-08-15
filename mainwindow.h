#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "formation.h"
#include "formateur.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ajouter_f_clicked();

    void on_supp_f_clicked();

    void on_modfi_f_clicked();

    void on_tableView_formations_doubleClicked(const QModelIndex &index);

    void on_ajouter_fr_clicked();

    void on_modfi_fr_clicked();

    void on_supp_fr_clicked();

    void on_tableView_formateur_doubleClicked(const QModelIndex &index);
    void populateFormateurComboBox();

    void on_lineEdit_textChanged( QString text);

    void on_tri_fr_clicked();
    void generatePDF( QString filePath);

    void on_ajouter_fr_2_clicked();


    void on_lineEdit_2_textChanged( QString text );

    void generatePDFf( QString filePath);
    void on_pdf_f_clicked();
    void displaystat();
    void markFormationDates();

    void on_calendarWidget_selectionChanged();
    void sendReminderEmails();







    void on_tri_f_clicked();

    void on_dateEdit_recherche_dateChanged(const QDate &date);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
   Formation formation;
   Formateur formateur;
};
#endif // MAINWINDOW_H
