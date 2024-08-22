#ifndef FORMATEUR_H
#define FORMATEUR_H

#include <QString>
#include <QSqlQueryModel>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QSqlError>
using namespace QtCharts;

class Formateur {
public:
    Formateur();
    Formateur(int id, QString nom, QString email, QString telephone, QString specialite, int experience);
    Formateur(QString nom, QString email, QString telephone, QString specialite, int experience);

    // Getter methods
    int getID() const;
    QString getNom() const;
    QString getEmail() const;
    QString getTelephone() const;
    QString getSpecialite() const;
    int getExperience() const;

    // Setter methods
    void setID(int id);
    void setNom(QString nom);
    void setEmail(QString email);
    void setTelephone(QString telephone);
    void setSpecialite(QString specialite);
    void setExperience(int experience);

    // Database operations
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier(int id);
    static QSqlQueryModel* rechercherParNom( QString nom);
    QSqlQueryModel *tri(const QString &columnName, Qt::SortOrder order);
    QBarSeries* getStatSpecialites();


private:
    int id;
    QString nom;
    QString email;
    QString telephone;
    QString specialite;
    int experience;
};

#endif // FORMATEUR_H
