/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "password_dialog.h"
#include "zulucrypt.h"

#include <Qt>
#include <QObject>
#include <QFileDialog>
#include <QDir>
#include <iostream>

password_Dialog::password_Dialog(QWidget *parent ) : QDialog(parent)
{
	passphraseDialogUI.setupUi(this);
	this->setFixedSize(this->size());

	connect(passphraseDialogUI.PushButtonCancel,SIGNAL(clicked()),this,SLOT(hideDialog())) ;
	connect(passphraseDialogUI.PushButtonOpen,SIGNAL(clicked()),this,SLOT(buttomOpenClicked())) ;
	connect(passphraseDialogUI.PushButtonMountPointPath,SIGNAL(clicked()),this,SLOT(mount_point()));
	connect(passphraseDialogUI.PushButtonVolumePath,SIGNAL(clicked()),this,SLOT(file_path())) ;

}

void password_Dialog::hideDialog(void )
{
	passphraseDialogUI.MountPointPath->setText(mount_point_path);
	passphraseDialogUI.OpenVolumePath->setText(volume_path);
	passphraseDialogUI.PassPhraseField->setText(passphrase);
	this->hide() ;
}

void password_Dialog::clearAllFields()
{
	passphraseDialogUI.MountPointPath->clear();
	passphraseDialogUI.OpenVolumePath->clear();
	passphraseDialogUI.PassPhraseField->clear();
	mount_point_path.clear();
	volume_path.clear();
	passphrase.clear();

}

void password_Dialog::clickedPartitionOption(QString option)
{
	passphraseDialogUI.OpenVolumePath->setText(option);

	if( this->isHidden() )
		this->show();

}

void password_Dialog::showDialog()
{

    if(this->isHidden())
	this->show();
}

void password_Dialog::mount_point(void )
{	
	mount_point_path = QFileDialog::getExistingDirectory((QWidget *) this,QString("Select Path to mount point folder"),QDir::homePath(),QFileDialog::ShowDirsOnly) ;
	passphraseDialogUI.MountPointPath->setText(mount_point_path);
}

void password_Dialog::file_path(void )
{	
	volume_path = QFileDialog::getOpenFileName((QWidget *) this,QString("Select encrypted volume"),QDir::homePath(),0);
	passphraseDialogUI.OpenVolumePath->setText(volume_path);
}

void password_Dialog::buttomOpenClicked(void )
{
	volume_path = passphraseDialogUI.OpenVolumePath->toPlainText() ;
	passphrase = passphraseDialogUI.PassPhraseField->text();
	mount_point_path = passphraseDialogUI.MountPointPath->toPlainText() ;

	if ( ( QString::compare(mount_point_path,QString("."))) == 0 ) {
		mount_point_path = QDir::currentPath() ;
	}
	if ( passphraseDialogUI.checkBoxReadOnly->isChecked() )
		mode = "ro" ;
	else
		mode = "rw" ;

	hideDialog();
}

password_Dialog::~password_Dialog()
{

}

