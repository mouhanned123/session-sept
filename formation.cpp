#include "formation.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QSqlError>

// Constructors
Formation::Formation() {}

Formation::Formation(int id, QString titre, QString description, int duree, QString date_de_debut, QDate journee, int formateur_id)
    : id(id), titre(titre), description(description), duree(duree), date_de_debut(date_de_debut), journee(journee), formateur_id(formateur_id) {}

Formation::Formation(QString titre, QString description, int duree, QString date_de_debut, QDate journee, int formateur_id)
    : titre(titre), description(description), duree(duree), date_de_debut(date_de_debut), journee(journee), formateur_id(formateur_id) {}

// Getters
int Formation::getID() {
    return id;
}

QString Formation::getTitre() {
    return titre;
}

QString Formation::getDescription() {
    return description;
}

int Formation::getDuree() {
    return duree;
}

QString Formation::getDateDeDebut() {
    return date_de_debut;
}

QDate Formation::getJournee() {
    return journee;
}

int Formation::getFormateurID() {
    return formateur_id;
}

// Setters
void Formation::setID(int id) {
    this->id = id;
}

void Formation::setTitre(QString titre) {
    this->titre = titre;
}

void Formation::setDescription(QString description) {
    this->description = description;
}

void Formation::setDuree(int duree) {
    this->duree = duree;
}

void Formation::setDateDeDebut(QString date_de_debut) {
    this->date_de_debut = date_de_debut;
}

void Formation::setJournee(QDate journee) {
    this->journee = journee;
}

void Formation::setFormateurID(int formateur_id) {
    this->formateur_id = formateur_id;
}

// Database methods
bool Formation::ajouter() {
    QSqlQuery query;

    query.prepare("INSERT INTO formation(titre, description, duree, date_de_debut, journee, id_formateur) "
                  "VALUES (:titre, :description, :duree, :date_de_debut, :journee, :id_formateur)");

    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":duree", duree);
    query.bindValue(":date_de_debut", date_de_debut);
    query.bindValue(":journee", journee);
    query.bindValue(":id_formateur", formateur_id);

    return query.exec();
}

QSqlQueryModel* Formation::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery("SELECT f.id, f.titre, f.description, f.duree, f.date_de_debut, f.journee, frm.nom "
                    "FROM formation f "
                    "JOIN formateur frm ON f.id_formateur = frm.id "
                    "ORDER BY f.id");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Titre"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Description"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Durée"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date de début"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Jour"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Formateur"));

    return model;
}

bool Formation::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM formation WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool Formation::modifier(int id) {
    QSqlQuery query;

    query.prepare("UPDATE formation SET titre = :titre, description = :description, duree = :duree, "
                  "date_de_debut = :date_de_debut, journee = :journee, id_formateur = :id_formateur WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":duree", duree);
    query.bindValue(":date_de_debut", date_de_debut);
    query.bindValue(":journee", journee);
    query.bindValue(":id_formateur", formateur_id);

    return query.exec();
}

QSqlQueryModel* Formation::rechercherParNom(QString recherche) {
    QSqlQueryModel* model = new QSqlQueryModel();
        QSqlQuery query;

        // Préparation de la requête pour rechercher par titre ou description
        query.prepare("SELECT * FROM formation WHERE titre LIKE :recherche OR description LIKE :recherche");
        query.bindValue(":recherche", "%" + recherche + "%");

        if (query.exec()) {
            model->setQuery(query);
        } else {
            qDebug() << "Failed to execute search query:" << query.lastError().text();
        }

        return model;
    }



QSqlQueryModel* Formation::Tri(QString cls, QString champ) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString queryString = "SELECT * FROM formation ORDER BY " + champ + " " + cls;
    QSqlQuery query;
    query.prepare(queryString);
    query.exec();
    model->setQuery(query);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID_FORMATION"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("TITRE"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("DESCRIPTION"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("DATE_DE_DEBUT"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("DUREE"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("JOURNEE"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("ID_FORMATEUR"));

    return model;
}

QBarSeries* Formation::getStatFormateurs() {
    QBarSeries* barSeries = new QBarSeries();

    QSqlQuery query;
    query.prepare("SELECT frm.nom, COUNT(f.id) AS nombre "
                  "FROM formation f "
                  "JOIN formateur frm ON f.id_formateur = frm.id "
                  "GROUP BY frm.nom "
                  "ORDER BY frm.nom;");

    if(query.exec()) {
        while (query.next()) {
            QString formateurNom = query.value(0).toString();
            int nombreFormations = query.value(1).toInt();

            QBarSet *barSet = new QBarSet(formateurNom);
            *barSet << nombreFormations;
            barSeries->append(barSet);
        }

        qDebug() << "Query executed successfully.";
    } else {
        qDebug() << "Query execution failed:" << query.lastError();
    }

    return barSeries;
}
QSqlQueryModel* Formation::rechercherParJournee(QDate journee) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;

    // Check if the provided date is valid
    if (!journee.isValid()) {
        qDebug() << "Invalid date provided.";
        return model;
    }

    // Prepare the query to search by journee with a correct date format
    query.prepare("SELECT * FROM formation WHERE journee = :journee");

    // Bind the date as a QDate
    query.bindValue(":journee", journee);

    // Execute the query and set the result model
    if (query.exec()) {
        model->setQuery(query);
    } else {
        qDebug() << "Failed to execute query:" << query.lastError().text();
    }

    return model;
}
