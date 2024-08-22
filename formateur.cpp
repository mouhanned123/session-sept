#include "formateur.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>


// Constructeurs
Formateur::Formateur() {}

Formateur::Formateur(int id, QString nom, QString email, QString telephone, QString specialite, int experience)
    : id(id), nom(nom), email(email), telephone(telephone), specialite(specialite), experience(experience) {}

Formateur::Formateur(QString nom, QString email, QString telephone, QString specialite, int experience)
    : nom(nom), email(email), telephone(telephone), specialite(specialite), experience(experience) {}

// Getters
int Formateur::getID() const {
    return id;
}

QString Formateur::getNom() const {
    return nom;
}

QString Formateur::getEmail() const {
    return email;
}

QString Formateur::getTelephone() const {
    return telephone;
}

QString Formateur::getSpecialite() const {
    return specialite;
}

int Formateur::getExperience() const {
    return experience;
}

// Setters
void Formateur::setID(int id) {
    this->id = id;
}

void Formateur::setNom(QString nom) {
    this->nom = nom;
}

void Formateur::setEmail(QString email) {
    this->email = email;
}

void Formateur::setTelephone(QString telephone) {
    this->telephone = telephone;
}

void Formateur::setSpecialite(QString specialite) {
    this->specialite = specialite;
}

void Formateur::setExperience(int experience) {
    this->experience = experience;
}

// Méthodes de gestion de la base de données
bool Formateur::ajouter(){
    QSqlQuery query;

    query.prepare("INSERT INTO formateur (nom, email, telephone, specialite, experience) "
                  "VALUES (:nom, :email, :telephone, :specialite, :experience)");

    query.bindValue(":nom", nom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":experience", experience);

    return query.exec();
}

QSqlQueryModel* Formateur::afficher() {
    QSqlQueryModel * model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM formateur ORDER BY id");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Expérience"));

    return model;
}

bool Formateur::supprimer(int id){
    QSqlQuery query;
    query.prepare("DELETE FROM formateur WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool Formateur::modifier(int id){
    QSqlQuery query;

    query.prepare("UPDATE formateur SET nom = :nom, email = :email, telephone = :telephone, "
                  "specialite = :specialite, experience = :experience WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":experience", experience);

    return query.exec();
}

QSqlQueryModel* Formateur::rechercherParNom(QString terme) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;

    // Préparer la requête pour rechercher par nom, email ou numéro de téléphone
    query.prepare("SELECT * FROM formateur WHERE nom LIKE :terme OR email LIKE :terme OR telephone LIKE :terme");
    query.bindValue(":terme", "%" + terme + "%");

    if (query.exec()) {
        model->setQuery(query);
    } else {
        qDebug() << "Failed to execute search query:" << query.lastError().text();
    }

    return model;
}
QSqlQueryModel* Formateur::tri(const QString& columnName, Qt::SortOrder order)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString sortOrder = (order == Qt::AscendingOrder) ? "ASC" : "DESC";
    QString queryStr = "SELECT * FROM formateur ORDER BY " + columnName + " " + sortOrder;
    model->setQuery(queryStr);
    return model;
}
QBarSeries* Formateur::getStatSpecialites() {
    QSqlQuery query;
    query.prepare("SELECT specialite, COUNT(*) FROM formateur GROUP BY specialite");

    if (!query.exec()) {
        qDebug() << "Failed to retrieve statistics data from the database:" << query.lastError();
        return nullptr;
    }

    QBarSeries *series = new QBarSeries();

    while (query.next()) {
        QString specialite = query.value(0).toString();
        int count = query.value(1).toInt();

        QBarSet *set = new QBarSet(specialite);
        *set << count;
        series->append(set);
    }
    return series;
}
