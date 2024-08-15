#ifndef FORMATION_H
#define FORMATION_H

#include <QString>
#include <QSqlQueryModel>
#include <QDate>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QSqlError>
using namespace QtCharts;

class Formation {
public:
    Formation();
    Formation(int id, QString titre, QString description, int duree, QString date_de_debut, QDate journee, int formateur_id);
    Formation(QString titre, QString description, int duree, QString date_de_debut, QDate journee, int formateur_id);

    // Getter methods
    int getID();
    QString getTitre();
    QString getDescription();
    int getDuree();
    QString getDateDeDebut();
    QDate getJournee();
    int getFormateurID();  // New getter for formateur_id

    // Setter methods
    void setID(int id);
    void setTitre(QString titre);
    void setDescription(QString description);
    void setDuree(int duree);
    void setDateDeDebut(QString date_de_debut);
    void setJournee(QDate journee);
    void setFormateurID(int formateur_id);  // New setter for formateur_id

    // Database operations
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier(int id);
    static QSqlQueryModel* rechercherParNom( QString nom);
    QSqlQueryModel *tri(const QString &columnName, Qt::SortOrder order);
    QBarSeries* getStatFormateurs();
    static QSqlQueryModel* rechercherParJournee(QDate journee);

private:
    int id;
    QString titre;
    QString description;
    int duree;
    QString date_de_debut;
    QDate journee;
    int formateur_id;  // New attribute
};

#endif // FORMATION_H
