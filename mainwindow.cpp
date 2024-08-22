#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formateur.h"
#include "formation.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>



#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QTextStream>
#include "smtp.h"





QT_CHARTS_USE_NAMESPACE



Formation formation;
Formateur formateur;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView_formations->setModel(formation.afficher());
    ui->tableView_formateur->setModel(formateur.afficher());
    populateSpecialiteComboBox();
    populateFormateurComboBox();

    displaystat();
    displaystatfr();
    markFormationDates();
    connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &MainWindow::on_calendarWidget_selectionChanged);


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::populateSpecialiteComboBox() {
    ui->comboBox_specialite->addItem("Developpement Web");
    ui->comboBox_specialite->addItem("Data Science");
    ui->comboBox_specialite->addItem("Reseaux et Securite");
    ui->comboBox_specialite->addItem("Gestion de projet");
    ui->comboBox_specialite->addItem("Intelligence Artificielle");
    ui->comboBox_specialite->addItem("Cloud Computing");
    ui->comboBox_specialite->addItem("Marketing Digital");
    ui->comboBox_specialite->addItem("Design et UX/UI");
    ui->comboBox_specialite->addItem("Ressources Humaines");
    ui->comboBox_specialite->addItem("Finance et Comptabilite");
}


void MainWindow::on_ajouter_f_clicked()
{
    // Récupérer les valeurs des champs de l'interface utilisateur
        QString titre = ui->lineEdit_titre->text();
        QString description = ui->lineEdit_description->text();
        int duree = ui->spinBox_duree->value();
        QString date_de_debut = ui->timeEdit_date_de_debut->time().toString("HH:mm"); // Assurez-vous que c'est QTimeEdit
        QDate journee = ui->dateEdit_jour->date(); // Assurez-vous que c'est QDateEdit

        // Récupérer l'id du formateur sélectionné dans le QComboBox
        int formateur_id = ui->comboBox_formateur->currentData().toInt();

        // Vérifier si tous les champs sont remplis
        if (titre.isEmpty() || description.isEmpty() || duree == 0 || formateur_id == 0) {
            QMessageBox::warning(this, "Champ(s) manquant(s)", "Veuillez remplir tous les champs.");
            return;
        }

        // Vérifier la validité de la date
        QDate aujourdhui = QDate::currentDate();
        if (!journee.isValid() || journee < aujourdhui) {
            QMessageBox::warning(this, "Date invalide", "La date doit être valide et ne peut pas être antérieure à aujourd'hui.");
            return;
        }

        qDebug() << "Titre:" << titre;
        qDebug() << "Description:" << description;
        qDebug() << "Durée:" << duree;
        qDebug() << "Date de début:" << date_de_debut;
        qDebug() << "Journee:" << journee;
        qDebug() << "Formateur ID:" << formateur_id;

        // Créer une instance de la classe Formation avec les données récupérées
        Formation formation(titre, description, duree, date_de_debut, journee, formateur_id);

        // Appeler la méthode ajouter() pour insérer les données dans la base de données
        bool success = formation.ajouter();

        // Afficher un message en fonction du résultat de l'opération
        if (success) {

            markFormationDates();
            displaystat();
            sendReminderEmails();
            QMessageBox::information(this, "Succès", "La formation a été ajoutée avec succès.");
            // Optionnel : Effacer les champs après ajout
            ui->lineEdit_titre->clear();
            ui->lineEdit_description->clear();
            ui->spinBox_duree->setValue(0);
            ui->timeEdit_date_de_debut->setTime(QTime::currentTime());
            ui->dateEdit_jour->setDate(QDate::currentDate());
            ui->comboBox_formateur->setCurrentIndex(-1);
            // Optionnel : Mettre à jour l'affichage des formations
            ui->tableView_formations->setModel(formation.afficher());

        } else {
            QMessageBox::critical(this, "Erreur", "Une erreur est survenue lors de l'ajout de la formation.");
        }
}

void MainWindow::on_supp_f_clicked()
{
    // at least one selected index
        QModelIndexList selectedIndexes = ui->tableView_formations->selectionModel()->selectedIndexes();

        if (!selectedIndexes.isEmpty()) {
            // get id from selected row
            int id = selectedIndexes.at(0).sibling(selectedIndexes.at(0).row(), 0).data().toInt();

            // Call delete function
            bool supprime = formation.supprimer(id);

            if (supprime) {
                displaystat();
                ui->tableView_formations->setModel(formation.afficher());
                QMessageBox::information(this, "Suppression réussie", "Formation supprimée avec succès.");
            } else {
                QMessageBox::warning(this, "Échec de la suppression", "Échec de la suppression de la formation.");
            }
        } else {
            QMessageBox::warning(this, "Aucune formation sélectionnée", "Veuillez sélectionner une formation à supprimer.");
        }
}

void MainWindow::on_modfi_f_clicked()
{
    bool ok;
        int id = ui->idFormation->text().toInt(&ok);
        if (!ok || id <= 0) {
            QMessageBox::critical(this, "Erreur", "ID de formation invalide.");
            return;
        }

        QString titre = ui->lineEdit_titre->text();
        QString description = ui->lineEdit_description->text();
        int duree = ui->spinBox_duree->value();
        QString date_de_debut = ui->timeEdit_date_de_debut->time().toString("HH:mm");; // Utilisation de QTime pour l'heure de début
        QDate journee = ui->dateEdit_jour->date(); // Utilisation de QDate pour la date de fin

        // Récupérer l'id du formateur sélectionné dans le QComboBox
        int formateur_id = ui->comboBox_formateur->currentData().toInt();

        // Validation des champs
        if (titre.isEmpty() || description.isEmpty() || date_de_debut.isNull() || journee.isNull() || formateur_id == 0) {
            QMessageBox::critical(this, "Erreur", "Veuillez remplir tous les champs.");
            return;
        }

        // Créer une instance de la classe Formation avec les données récupérées
        Formation formation(id, titre, description, duree, date_de_debut, journee, formateur_id);

        // Appeler la méthode modifier() pour mettre à jour les données dans la base de données
        bool success = formation.modifier(id);

        if (success) {
            populateSpecialiteComboBox();
            markFormationDates(); // Si cette méthode est nécessaire
            displaystat(); // Si cette méthode est nécessaire
            // Mettre à jour l'affichage des formations
            ui->tableView_formations->setModel(formation.afficher());

            // Effacer les champs après modification
            ui->idFormation->clear();
            ui->lineEdit_titre->clear();
            ui->lineEdit_description->clear();
            ui->spinBox_duree->setValue(0);
            ui->timeEdit_date_de_debut->setTime(QTime::currentTime()); // Réinitialiser le QTime
            ui->dateEdit_jour->setDate(QDate::currentDate());
            ui->comboBox_formateur->setCurrentIndex(-1); // Réinitialiser le QComboBox


            QMessageBox::information(this, "Modification réussie", "Formation modifiée avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur", "Une erreur est survenue lors de la modification de la formation.");
        }
}

void MainWindow::on_tableView_formations_doubleClicked(const QModelIndex &index)
{
    int row = index.row();

        // Récupérer les données de la ligne sélectionnée
        int id = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 0)).toInt();
        QString titre = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 1)).toString();
        QString description = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 2)).toString();
        int duree = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 3)).toInt();
        QTime dateDebut = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 4)).toTime(); // Utilisation de QTime pour l'heure de début
        QDate journee = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 5)).toDate(); // Utilisation de QDate pour la date de fin
        int formateur_id = ui->tableView_formations->model()->data(ui->tableView_formations->model()->index(row, 6)).toInt(); // Assuming formateur_id is the 6th column

        // Définir les données dans les éléments UI respectifs
        ui->idFormation->setText(QString::number(id));
        ui->lineEdit_titre->setText(titre);
        ui->lineEdit_description->setText(description);
        ui->spinBox_duree->setValue(duree);
        ui->timeEdit_date_de_debut->setTime(dateDebut); // Utilisation de QTime
        ui->dateEdit_jour->setDate(journee); // Utilisation de QDate

        // Définir le formateur dans le QComboBox
        int indexComboBox = ui->comboBox_formateur->findData(formateur_id);
        if (indexComboBox != -1) {
            ui->comboBox_formateur->setCurrentIndex(indexComboBox);
        }
}

void MainWindow::on_ajouter_fr_clicked()
{
    // Récupérer les valeurs des champs de l'interface utilisateur
        QString nom = ui->lineEdit_nom->text();
        QString email = ui->lineEdit_mail->text();
        QString telephone = ui->lineEdit_tel->text();
        QString specialite = ui->comboBox_specialite->currentText();;
        int experience = ui->spinBox_exp->value();

        // Vérifier si tous les champs sont remplis
        if (nom.isEmpty() || email.isEmpty() || telephone.isEmpty() || specialite.isEmpty() || experience == 0) {
            QMessageBox::warning(this, "Champ(s) manquant(s)", "Veuillez remplir tous les champs.");
            return;
        }

        // Créer une instance de la classe Formateur avec les données récupérées
        Formateur formateur(nom, email, telephone, specialite, experience);

        // Appeler la méthode ajouter() pour insérer les données dans la base de données
        bool success = formateur.ajouter();

        // Afficher un message en fonction du résultat de l'opération
        if (success) {


            QMessageBox::information(this, "Succès", "Le formateur a été ajouté avec succès.");
            // Optionnel : Effacer les champs après ajout
            ui->lineEdit_nom->clear();
            ui->lineEdit_mail->clear();
            ui->lineEdit_tel->clear();
            ui->comboBox_specialite->clear();
            ui->spinBox_exp->setValue(0);
            // Optionnel : Mettre à jour l'affichage des formateurs
            ui->tableView_formateur->setModel(formateur.afficher());
            populateFormateurComboBox();
        } else {
            QMessageBox::critical(this, "Erreur", "Une erreur est survenue lors de l'ajout du formateur.");
        }

}

void MainWindow::on_modfi_fr_clicked()
{
    int id = ui->idFormateur->text().toInt();
        QString nom = ui->lineEdit_nom->text();
        QString email = ui->lineEdit_mail->text();
        QString telephone = ui->lineEdit_tel->text();
        QString specialite = ui->comboBox_specialite->currentText();
        int experience = ui->spinBox_exp->value();

        // Validation des champs
        if (nom.isEmpty() || email.isEmpty() || telephone.isEmpty() || specialite.isEmpty()) {
            QMessageBox::critical(nullptr, QObject::tr("Validation Error"),
                                  QObject::tr("Veuillez remplir tous les champs."),
                                  QMessageBox::Ok);
            return;
        }

        Formateur formateur(id, nom, email, telephone, specialite, experience);

        bool success = formateur.modifier(id);

        if (success) {
            ui->tableView_formateur->setModel(formateur.afficher());

            ui->idFormateur->clear();
            ui->lineEdit_nom->clear();
            ui->lineEdit_mail->clear();
            ui->lineEdit_tel->clear();
            ui->comboBox_specialite->clear();
            ui->spinBox_exp->setValue(0);
            populateSpecialiteComboBox();

            QMessageBox::information(nullptr, QObject::tr("Modifier un formateur"),
                                     QObject::tr("Formateur modifié avec succès.\n"
                                                 "Cliquez sur Annuler pour quitter."),
                                     QMessageBox::Cancel);
        } else {
            QMessageBox::critical(nullptr, QObject::tr("ERREUR"),
                                  QObject::tr("Une erreur est survenue.\n"
                                              "Cliquez sur Annuler pour quitter."),
                                  QMessageBox::Cancel);
        }
}

void MainWindow::on_supp_fr_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView_formateur->selectionModel()->selectedIndexes();

        if (!selectedIndexes.isEmpty()) {
            // get id from selected row
            int id = selectedIndexes.at(0).sibling(selectedIndexes.at(0).row(), 0).data().toInt();

            // Call delete function
            bool supprime = formateur.supprimer(id);

            if (supprime) {
                ui->tableView_formateur->setModel(formateur.afficher());
                QMessageBox::information(this, "Suppression réussie", "Formateur supprimé avec succès.");
            } else {
                QMessageBox::warning(this, "Échec de la suppression", "Échec de la suppression du formateur.");
            }
        } else {
            QMessageBox::warning(this, "Aucun formateur sélectionné", "Veuillez sélectionner un formateur à supprimer.");
        }
}

void MainWindow::on_tableView_formateur_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
       int id = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 0)).toInt();
       QString nom = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 1)).toString();
       QString email = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 2)).toString();
       QString telephone = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 3)).toString();
       QString specialite = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 4)).toString();
       int experience = ui->tableView_formateur->model()->data(ui->tableView_formateur->model()->index(row, 5)).toInt();

       ui->idFormateur->setText(QString::number(id));
       ui->lineEdit_nom->setText(nom);
       ui->lineEdit_mail->setText(email);
       ui->lineEdit_tel->setText(telephone);
       ui->comboBox_specialite->setCurrentText(specialite);
       ui->spinBox_exp->setValue(experience);
}

void MainWindow::populateFormateurComboBox() {
    QSqlQuery query("SELECT id, nom FROM formateur");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->comboBox_formateur->addItem(name, id);
    }
}

void MainWindow::on_lineEdit_textChanged(QString text)
{
    QSqlQueryModel* resultModel = Formateur::rechercherParNom(text);

    if (resultModel) {
        ui->tableView_formateur->setModel(resultModel);
    } else {
        // Gérer l'erreur, par exemple afficher un message d'erreur
        QMessageBox::critical(this, "Erreur de recherche", "Une erreur s'est produite lors de la recherche.");
    }
}

void MainWindow::on_tri_fr_clicked()
{
    // Call the sorting method in your Fournisseur class
        QSqlQueryModel *sortedModel = formateur.tri("terme", Qt::AscendingOrder);

        // Set the sorted model to the table view
        ui->tableView_formateur->setModel(sortedModel);
}
void MainWindow::generatePDF( QString filePath)
{
    // Get the file path using QFileDialog
    QString fileName = filePath;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, "Save PDF", QDir::homePath(), "PDF Files (*.pdf)");
    }

    // Check if the user canceled the dialog or didn't provide a file name
    if (fileName.isNull()) {
        qDebug() << "PDF generation canceled by the user.";
        return;
    }

    // Create a PDF writer
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A1));

    // Create a painter for the PDF
    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Retrieve data from the table view model and write it to the PDF
    int rowCount = ui->tableView_formateur->model()->rowCount();
    int colCount = ui->tableView_formateur->model()->columnCount();
    int cellWidth = pdfWriter.width() / colCount;
    int cellHeight = 500;
    int margin = 100;


    QFont font = painter.font();
    font.setPixelSize(200);
    painter.setFont(font);

    // Draw headers
    for (int col = 0; col < colCount; ++col) {
        QString headerData = ui->tableView_formateur->model()->headerData(col, Qt::Horizontal).toString();
        int x = col * cellWidth + margin;
        int y = 0;
        QRect headerRect(x, y, cellWidth - margin, cellHeight - margin);
        painter.drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, headerData);
    }

    // Draw cell data
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QModelIndex index = ui->tableView_formateur->model()->index(row, col);
            QString cellData = ui->tableView_formateur->model()->data(index, Qt::DisplayRole).toString();

            // Calculate cell position with padding
            int x = col * cellWidth + margin;
            int y = (row + 1) * cellHeight + margin;

            // Draw cell data with padding
            QRect cellRect(x, y, cellWidth - margin, cellHeight - margin);
            painter.drawText(cellRect, Qt::AlignLeft | Qt::AlignVCenter, cellData);
        }
    }

    // Finish painting
    painter.end();

    QMessageBox::information(this, "PDF Generated", "PDF generated successfully at:\n" + fileName);
}

void MainWindow::on_ajouter_fr_2_clicked()
{
    generatePDF("");
}

void MainWindow::on_lineEdit_2_textChanged( QString text )
{
    QSqlQueryModel* resultModel = Formation::rechercherParNom(text);

    if (resultModel) {
        ui->tableView_formations->setModel(resultModel);
    } else {
        // Gérer l'erreur, par exemple afficher un message d'erreur
        QMessageBox::critical(this, "Erreur de recherche", "Une erreur s'est produite lors de la recherche.");
    }
}

void MainWindow::generatePDFf( QString filePath)
{
    // Get the file path using QFileDialog
    QString fileName = filePath;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, "Save PDF", QDir::homePath(), "PDF Files (*.pdf)");
    }

    // Check if the user canceled the dialog or didn't provide a file name
    if (fileName.isNull()) {
        qDebug() << "PDF generation canceled by the user.";
        return;
    }

    // Create a PDF writer
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A1));

    // Create a painter for the PDF
    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Retrieve data from the table view model and write it to the PDF
    int rowCount = ui->tableView_formations->model()->rowCount();
    int colCount = ui->tableView_formations->model()->columnCount();
    int cellWidth = pdfWriter.width() / colCount;
    int cellHeight = 500;
    int margin = 100;


    QFont font = painter.font();
    font.setPixelSize(200);
    painter.setFont(font);

    // Draw headers
    for (int col = 0; col < colCount; ++col) {
        QString headerData = ui->tableView_formations->model()->headerData(col, Qt::Horizontal).toString();
        int x = col * cellWidth + margin;
        int y = 0;
        QRect headerRect(x, y, cellWidth - margin, cellHeight - margin);
        painter.drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, headerData);
    }

    // Draw cell data
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QModelIndex index = ui->tableView_formations->model()->index(row, col);
            QString cellData = ui->tableView_formations->model()->data(index, Qt::DisplayRole).toString();

            // Calculate cell position with padding
            int x = col * cellWidth + margin;
            int y = (row + 1) * cellHeight + margin;

            // Draw cell data with padding
            QRect cellRect(x, y, cellWidth - margin, cellHeight - margin);
            painter.drawText(cellRect, Qt::AlignLeft | Qt::AlignVCenter, cellData);
        }
    }

    // Finish painting
    painter.end();

    QMessageBox::information(this, "PDF Generated", "PDF generated successfully at:\n" + fileName);
}


void MainWindow::on_pdf_f_clicked()
{
   generatePDFf("") ;
}

void MainWindow::displaystat() {
    // Create a QVBoxLayout for ui->frameF if it doesn't have a layout
    if (!ui->frameF->layout()) {
        QVBoxLayout *frameLayout = new QVBoxLayout(ui->frameF);
        ui->frameF->setLayout(frameLayout);
    }

    // Clear any existing widgets in the QFrame
    QLayoutItem *item;
    while ((item = ui->frameF->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Retrieve the data to be displayed in the bar chart
    Formation f;
    QBarSeries *series = f.getStatFormateurs();

    if (!series) {
        qDebug() << "Failed to retrieve statistics data from the database.";
        return;
    }

    // Create the bar chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de formations par formateur");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create the category axis (X-axis)
    QBarCategoryAxis *axisX = new QBarCategoryAxis;
    axisX->setTitleText("Formateur");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create the value axis (Y-axis)
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Nombre de formations");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Create the chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing, true);

    // Adjust the size of the chart view
    chartView->setFixedSize(721, 281);

    // Add the chart view to the layout of the parent widget
    ui->frameF->layout()->addWidget(chartView);
}

void MainWindow::markFormationDates() {
    // Récupérer le widget calendrier
    QCalendarWidget *calendar = ui->calendarWidget;

    // Créer un format spécial pour marquer les dates
    QTextCharFormat format;
    format.setBackground(Qt::yellow);  // Changer la couleur de fond en jaune

    // Vérifier si le widget calendrier est correctement initialisé
    if (!calendar) {
        qDebug() << "Calendar widget not found!";
        return;
    }

    // Préparer la requête SQL pour récupérer les dates de début des formations
    QSqlQuery query;
    if (!query.prepare("SELECT journee FROM formation")) {
        qDebug() << "Query preparation failed:" << query.lastError().text();
        return;
    }

    // Exécuter la requête et vérifier si elle réussit
    if (!query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return;
    }

    // Parcourir les résultats de la requête
    while (query.next()) {
        QVariant dateValue = query.value(0);

        // Vérifier si la date est valide avant de l'utiliser
        if (!dateValue.isValid() || !dateValue.canConvert<QDate>()) {
            qDebug() << "Invalid date value retrieved:" << dateValue;
            continue;
        }

        QDate startDate = dateValue.toDate();

        // Vérifier si la conversion en QDate s'est bien passée
        if (!startDate.isValid()) {
            qDebug() << "Failed to convert to QDate:" << dateValue;
            continue;
        }

        // Marquer la date dans le calendrier
        qDebug() << "Marking date:" << startDate;
        calendar->setDateTextFormat(startDate, format);
    }
}


void MainWindow::on_calendarWidget_selectionChanged()
{
    // Récupérer la date sélectionnée dans le calendrier
        QDate selectedDate = ui->calendarWidget->selectedDate();

        // Convertir la QDate en une chaîne formatée selon le format attendu par Oracle
        QString formattedDate = selectedDate.toString("dd-MMM-yy").toUpper();

        // Préparer la requête pour récupérer les formations dont la journee correspond à la date sélectionnée
        QSqlQuery query;
        query.prepare("SELECT f.id, f.titre, f.description, f.duree, f.date_de_debut, formateur.nom "
                      "FROM formation f "
                      "JOIN formateur ON f.id_formateur = formateur.id "
                      "WHERE TO_CHAR(f.journee, 'DD-MON-YY') = :selectedDate");
        query.bindValue(":selectedDate", formattedDate);

        if (!query.exec()) {
            qDebug() << "Failed to retrieve formations:" << query.lastError().text();
            return;
        }

        // Créer un modèle pour afficher les données dans le QTableView
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(query);

        // Définir les en-têtes de colonnes
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("Titre"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("Description"));
        model->setHeaderData(3, Qt::Horizontal, QObject::tr("Durée"));
        model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date de début"));
        model->setHeaderData(5, Qt::Horizontal, QObject::tr("Formateur"));

        // Mettre à jour le QTableView avec le nouveau modèle
        ui->tableView_formations->setModel(model);
}

void MainWindow::sendReminderEmails() {
    QDate tomorrow = QDate::currentDate().addDays(1);

    // Préparer la requête pour récupérer les formations dont la journée est demain
    QSqlQuery query;
    query.prepare("SELECT f.titre, f.date_de_debut, f.duree, f.journee, f.id_formateur, formateur.nom, formateur.email "
                  "FROM formation f "
                  "JOIN formateur ON f.id_formateur = formateur.id "
                  "WHERE f.journee = :tomorrow");
    query.bindValue(":tomorrow", tomorrow);

    if (!query.exec()) {
        qDebug() << "Failed to retrieve formations:" << query.lastError().text();
        return;
    }

    // Envoyer un e-mail à chaque formateur
    while (query.next()) {
        QString titre = query.value("titre").toString();
        QString formateurNom = query.value("nom").toString();
        QString formateurEmail = query.value("email").toString();

        // Récupérer et formater les champs nécessaires
        QDate journee = query.value("journee").toDate();
        QString journeeStr = journee.toString("dd/MM/yyyy");

        QString dateDeDebut = query.value("date_de_debut").toString();




        QString duree = query.value("duree").toString();

        // Créer le contenu de l'e-mail
        QString subject = QString("Rappel: Formation '%1' Demain").arg(titre);
        QString body = QString("Bonjour %1,\n\n"
                               "Ceci est un rappel que vous avez une formation '%2' prévue pour demain.\n"
                               "Journée : %3\n"
                               "Heure de début : %4\n"
                               "Durée : %5\n\n"
                               "Merci,\n"
                               "L'équipe de gestion des formations.")
                               .arg(formateurNom)
                               .arg(titre)
                               .arg(journeeStr)  // Affiche la journée (date)
                               .arg(dateDeDebut)  // Affiche uniquement l'heure
                               .arg(duree);

        // Configurer et envoyer l'e-mail via un serveur SMTP
        Smtp *smtp = new Smtp("jery.wizin@gmail.com", "tqyb pqwu pzzl pchx", "smtp.gmail.com", 465);
        smtp->sendMail("jery.wizin@gmail.com", formateurEmail, subject, body);
    }
}














void MainWindow::on_tri_f_clicked()
{
    QString champ = "titre";  // Par défaut, on trie par titre
       QString order = "asc";    // Par défaut, on trie de manière ascendante

       if (ui->comboBoxsortchamp->currentText().compare("Journee") == 0) {
           champ = "journee";
       } else if (ui->comboBoxsortchamp->currentText().compare("Titre") == 0) {
           champ = "titre";
       }

       if (ui->comboBox_order->currentText().compare("Descendant") == 0) {
           order = "desc";
       }
       qDebug() << order ;
        qDebug() << champ ;

       ui->tableView_formations->setModel(formation.Tri(order, champ));
}

void MainWindow::on_dateEdit_recherche_dateChanged(const QDate &date)
{
    // Create an instance of the Formation class
       Formation formation;

       // Call the function to search formations by journee
       QSqlQueryModel* model = formation.rechercherParJournee(date);

       // Set the model to the QTableView
       ui->tableView_formations->setModel(model);
}

void MainWindow::on_pushButton_clicked()
{
     ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_2_clicked()
{
ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::displaystatfr() {
    // Create a QVBoxLayout for ui->frameF if it doesn't have a layout
    if (!ui->frameFR->layout()) {
        QVBoxLayout *frameLayout = new QVBoxLayout(ui->frameFR);
        ui->frameFR->setLayout(frameLayout);
    }

    // Clear any existing widgets in the QFrame
    QLayoutItem *item;
    while ((item = ui->frameFR->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Retrieve the data to be displayed in the bar chart
    Formateur f;
    QBarSeries *series = f.getStatSpecialites();

    if (!series) {
        qDebug() << "Failed to retrieve statistics data from the database.";
        return;
    }

    // Create the bar chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de formateurs par spécialité");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create the category axis (X-axis)
    QBarCategoryAxis *axisX = new QBarCategoryAxis;
    axisX->setTitleText("Spécialité");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create the value axis (Y-axis)
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Nombre de formateurs");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Create the chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing, true);

    // Adjust the size of the chart view
    chartView->setFixedSize(721, 281);

    // Add the chart view to the layout of the parent widget
    ui->frameFR->layout()->addWidget(chartView);
}

