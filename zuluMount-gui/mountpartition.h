
/*
 *
 *  Copyright (c) 2012
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOUNTPARTITION_H
#define MOUNTPARTITION_H

#include <QWidget>
#include <QString>

class QTableWidget ;
class QCloseEvent ;
class QMenu ;
class QAction ;

namespace Ui {
class mountPartition;
}

class mountPartition : public QWidget
{
	Q_OBJECT
public:
	explicit mountPartition( QWidget * parent = 0,QTableWidget * table = 0,const QString& folderOpener = QString(),bool b = true ) ;
	void ShowUI( QString path,QString label ) ;
	void HideUI( void ) ;
	void AutoMount( QStringList entry ) ;
	~mountPartition() ;
signals:
	void mounted( QString ) ;
	void autoMountComplete( void ) ;
	void cancel( void ) ;
private slots:
	void stateChanged( int ) ;
	void pbMount( void ) ;
	void pbOptions( void ) ;
	void pbCancel( void ) ;
	void pbOpenMountPath( void ) ;
	void showOffSetWindowOption( void ) ;
	void showFileSystemOptionWindow( void ) ;
	void slotMountComplete( int,QString ) ;
	void checkBoxReadOnlyStateChanged( int ) ;
	void fileManagerOpenStatus( int exitCode, int exitStatus,int startError ) ;
	void deviceOffSet( QString,QString ) ;
	void doAction( QAction * ) ;
private:
	void enableAll( void ) ;
	void disableAll( void ) ;
	void closeEvent( QCloseEvent * ) ;
	Ui::mountPartition * m_ui;
	QString m_path ;
	QString m_label ;
	QString m_point ;
	QString m_folderOpener ;
	QString m_deviceOffSet ;
	QString m_key ;
	QString m_options ;
	QTableWidget * m_table ;
	QMenu * m_menu ;
	bool m_autoOpenFolderOnMount ;
};

#endif // MOUNTPARTITION_H
