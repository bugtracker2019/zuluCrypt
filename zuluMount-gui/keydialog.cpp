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

#include "deviceoffset.h"

#include "keydialog.h"
#include "ui_keydialog.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
#include <QTableWidget>

#include "../zuluCrypt-gui/dialogmsg.h"
#include "task.h"
#include "../zuluCrypt-cli/constants.h"
#include "plugin_path.h"
#include "../zuluCrypt-gui/utility.h"
#include "../zuluCrypt-gui/lxqt_wallet/frontend/lxqt_wallet.h"
#include "mountoptions.h"

#define KWALLET         "kde wallet"
#define INTERNAL_WALLET "internal wallet"
#define GNOME_WALLET    "gnome wallet"

/*
 * this ugly global variable is defined in zulucrypt.cpp to prevent multiple prompts when opening multiple volumes
 */
static QString _internalPassWord ;

keyDialog::keyDialog( QWidget * parent,QTableWidget * table,const QString& path,
		      const QString& type,const QString& folderOpener,bool autoOpenFolderOnMount ) :
	QDialog( parent ),m_ui(new Ui::keyDialog)
{
	m_ui->setupUi( this ) ;
	m_ui->checkBoxShareMountPoint->setToolTip( utility::shareMountPointToolTip() ) ;
	m_table = table ;
	m_path = path ;
	m_working = false ;
	m_folderOpener = folderOpener ;

	m_autoOpenFolderOnMount = autoOpenFolderOnMount ;

	QString msg ;
	if( type == QString( "crypto_LUKS" ) ){
		msg = tr( "unlock and mount a luks volume in \"%1\"").arg( m_path ) ;
	}else{
		msg = tr( "unlock and mount an encrypted volume in \"%1\"").arg( m_path ) ;
	}
	this->setWindowTitle( msg ) ;

	m_ui->lineEditMountPoint->setText( path ) ;
	m_ui->pbOpenMountPoint->setIcon( QIcon( QString( ":/folder.png" ) ) ) ;

	m_menu = new QMenu( this ) ;

	this->setFixedSize( this->size() ) ;
	this->setWindowFlags( Qt::Window | Qt::Dialog ) ;
	this->setFont( parent->font() ) ;

	m_ui->lineEditKey->setFocus() ;

	m_ui->checkBoxOpenReadOnly->setChecked( utility::getOpenVolumeReadOnlyOption( QString( "zuluMount-gui" ) ) ) ;

	m_ui->pbkeyOption->setEnabled( false ) ;

	m_ui->lineEditKey->setEchoMode( QLineEdit::Password ) ;

	connect( m_ui->pbOptions,SIGNAL( clicked() ),this,SLOT( pbOptions() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) ) ;
	connect( m_ui->pbOpen,SIGNAL( clicked() ),this,SLOT( pbOpen() ) ) ;
	connect( m_ui->pbkeyOption,SIGNAL( clicked() ),this,SLOT( pbkeyOption() ) ) ;
	connect( m_ui->pbOpenMountPoint,SIGNAL( clicked() ),this,SLOT( pbMountPointPath() ) ) ;
	connect( m_ui->checkBoxOpenReadOnly,SIGNAL( stateChanged( int ) ),this,SLOT( cbMountReadOnlyStateChanged( int ) ) ) ;
	connect( m_ui->cbKeyType,SIGNAL( currentIndexChanged( int ) ),this,SLOT( cbActicated( int ) ) ) ;

	m_ui->pbOpenMountPoint->setVisible( false ) ;

	m_point = m_path.split( "/" ).last() ;
	m_ui->lineEditMountPoint->setText( m_point ) ;

	QAction * ac = new QAction( this ) ;
	QKeySequence s( Qt::CTRL + Qt::Key_F ) ;
	ac->setShortcut( s ) ;
	connect( ac,SIGNAL( triggered() ),this,SLOT( showOffSetWindowOption() ) ) ;
	this->addAction( ac ) ;

	m_menu_1 = new QMenu( this ) ;

	m_menu_1->addAction( tr( "set file system options" ) ) ;
	m_menu_1->addAction( tr( "set volume offset" ) ) ;

	connect( m_menu_1,SIGNAL( triggered( QAction * ) ),this,SLOT( doAction( QAction * ) ) ) ;

	this->installEventFilter( this ) ;
}

bool keyDialog::eventFilter( QObject * watched,QEvent * event )
{
	if( watched == this ){
		if( event->type() == QEvent::KeyPress ){
			QKeyEvent * keyEvent = static_cast< QKeyEvent* >( event ) ;
			if( keyEvent->key() == Qt::Key_Escape ){
				this->pbCancel() ;
				return true ;
			}
		}
	}

	return false ;
}

void keyDialog::pbOptions()
{
	m_menu_1->exec( QCursor::pos() ) ;
}

void keyDialog::showOffSetWindowOption()
{
	deviceOffset * d = new deviceOffset( this ) ;
	connect( d,SIGNAL( offSetValue( QString,QString ) ),this,SLOT( deviceOffSet( QString,QString ) ) ) ;
	d->ShowUI_1() ;
}

void keyDialog::showFileSystemOptionWindow()
{
	mountOptions * m = new mountOptions( &m_options,this ) ;
	m->ShowUI() ;
}

void keyDialog::doAction( QAction * ac )
{
	if( ac->text() == tr( "set file system options" ) ){
		this->showFileSystemOptionWindow() ;
	}else{
		this->showOffSetWindowOption() ;
	}
}

void keyDialog::deviceOffSet( QString deviceOffSet,QString key )
{
	m_deviceOffSet = QString( " -o %1" ).arg( deviceOffSet ) ;
	Q_UNUSED( key ) ;
}

void keyDialog::cbMountReadOnlyStateChanged( int state )
{
	m_ui->checkBoxOpenReadOnly->setEnabled( false ) ;
	m_ui->checkBoxOpenReadOnly->setChecked( utility::setOpenVolumeReadOnly( this,state == Qt::Checked,QString( "zuluMount-gui" ) ) ) ;
	m_ui->checkBoxOpenReadOnly->setEnabled( true ) ;
	if( m_ui->lineEditKey->text().isEmpty() ){
		m_ui->lineEditKey->setFocus() ;
	}else if( m_ui->lineEditMountPoint->text().isEmpty() ){
		m_ui->lineEditMountPoint->setFocus() ;
	}else{
		m_ui->pbOpen->setFocus() ;
	}
}

void keyDialog::pbMountPointPath()
{
	QString msg = tr( "select a folder to create a mount point in" ) ;
	QString Z = QFileDialog::getExistingDirectory( this,msg,QDir::homePath(),QFileDialog::ShowDirsOnly ) ;

	if( !Z.isEmpty() ){
		Z = Z + QString( "/" ) + m_ui->lineEditMountPoint->text().split( "/" ).last() ;
		m_ui->lineEditMountPoint->setText( Z ) ;
	}
}

void keyDialog::enableAll()
{
	m_ui->pbOptions->setEnabled( true ) ;
	m_ui->label_2->setEnabled( true ) ;
	m_ui->lineEditMountPoint->setEnabled( true ) ;
	m_ui->pbOpenMountPoint->setEnabled( true ) ;
	m_ui->pbCancel->setEnabled( true ) ;
	m_ui->pbOpen->setEnabled( true ) ;
	m_ui->label->setEnabled( true ) ;
	m_ui->cbKeyType->setEnabled( true ) ;
	if( m_ui->cbKeyType->currentIndex() != keyDialog::plugin ){
		m_ui->lineEditKey->setEnabled( true ) ;
	}
	m_ui->pbkeyOption->setEnabled( true ) ;
	m_ui->checkBoxOpenReadOnly->setEnabled( true ) ;
	m_ui->checkBoxShareMountPoint->setEnabled( true ) ;
}

void keyDialog::disableAll()
{
	m_ui->cbKeyType->setEnabled( false ) ;
	m_ui->pbOptions->setEnabled( false ) ;
	m_ui->pbkeyOption->setEnabled( false ) ;
	m_ui->label_2->setEnabled( false ) ;
	m_ui->lineEditMountPoint->setEnabled( false ) ;
	m_ui->pbOpenMountPoint->setEnabled( false ) ;
	m_ui->lineEditKey->setEnabled( false ) ;
	m_ui->pbCancel->setEnabled( false ) ;
	m_ui->pbOpen->setEnabled( false ) ;
	m_ui->label->setEnabled( false ) ;
	m_ui->checkBoxOpenReadOnly->setEnabled( false ) ;
	m_ui->checkBoxShareMountPoint->setEnabled( false ) ;
}

void keyDialog::KeyFile()
{
	if( m_ui->cbKeyType->currentIndex() == keyDialog::keyfile ){
		QString msg = tr( "select a file to be used as a keyfile" ) ;
		QString Z = QFileDialog::getOpenFileName( this,msg,QDir::homePath() ) ;

		if( !Z.isEmpty() ){
			m_ui->lineEditKey->setText( Z ) ;
		}
	}
}

void keyDialog::pbkeyOption()
{
	if( m_ui->cbKeyType->currentIndex() == keyDialog::plugin ){
		this->Plugin() ;
	}else if( m_ui->cbKeyType->currentIndex() == keyDialog::keyfile ){
		this->KeyFile() ;
	}
}

void keyDialog::Plugin()
{
	QStringList list ;

	// ZULUCRYPTpluginPath is set at config time and it equals $prefix/lib(64)/zuluCrypt

	QDir dir( QString( ZULUCRYPTpluginPath ) ) ;

	if( dir.exists() ){
		list = dir.entryList() ;
	}

	list.removeOne( QString( "zuluCrypt-testKey" ) ) ;
	list.removeOne( QString( "." ) ) ;
	list.removeOne( QString( ".." ) ) ;
	list.removeOne( "keyring" ) ;
	list.removeOne( "kwallet" ) ;

	if( LxQt::Wallet::backEndIsSupported( LxQt::Wallet::secretServiceBackEnd ) ){
		list.prepend( tr( GNOME_WALLET ) ) ;
	}
	if( LxQt::Wallet::backEndIsSupported( LxQt::Wallet::kwalletBackEnd ) ){
		list.prepend( tr( KWALLET ) ) ;
	}

	list.prepend( tr( INTERNAL_WALLET ) ) ;

	m_menu->clear() ;

	int j = list.size()  ;

	if( j == 0 ){
		return ;
	}
	for( int i = 0 ; i < j ; i++ ){
		m_menu->addAction( list.at( i ) ) ;
	}
	m_menu->addSeparator() ;

	m_menu->addAction( tr( "cancel" ) ) ;

	connect( m_menu,SIGNAL( triggered( QAction * ) ),this,SLOT( pbPluginEntryClicked( QAction * ) ) ) ;

	m_menu->exec( QCursor::pos() ) ;
}

void keyDialog::pbPluginEntryClicked( QAction * e )
{
	if( e->text() != tr( "cancel" ) ){
		m_ui->lineEditKey->setText( e->text() ) ;
	}
}

void keyDialog::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbCancel() ;
}

void keyDialog::fileManagerOpenStatus( int exitCode,int exitStatus,int startError )
{
	Q_UNUSED( startError ) ;
	if( exitCode != 0 || exitStatus != 0 ){
		DialogMsg msg( this ) ;
		msg.ShowUIOK( tr( "warning" ),tr( "could not open mount point because \"%1\" tool does not appear to be working correctly").arg( m_folderOpener ) ) ;
	}
}

void keyDialog::slotMountComplete( int st,QString m )
{
	m_working = false ;

	if( st == 12 && m_ui->cbKeyType->currentIndex() == keyDialog::plugin ){
		/*
		 * A user cancelled the plugin
		 */
		return this->enableAll() ;
	}

	if( st == 0 ){
		if( utility::mapperPathExists( m_path ) ) {
			/*
			 * The volume is reported as opened and it actually is
			 */
			if( m_autoOpenFolderOnMount ){
				Task * t = new Task() ;
				t->setMountPoint( utility::mountPath( m_point ) ) ;
				t->setMountPointOpener( m_folderOpener ) ;
				connect( t,SIGNAL( errorStatus( int,int,int ) ),this,SLOT( fileManagerOpenStatus( int,int,int ) ) ) ;
				t->start( Task::openMountPoint ) ;
			}
		}else{
			/*
			 * The volume is reported as opened but it isnt,possible reason is a backe end crash
			 */

			DialogMsg m( this ) ;

			m.ShowUIOK( tr( "ERROR" ),tr( "An error has occured and the volume could not be opened" ) ) ;
			emit cancel() ;
		}
		this->HideUI() ;
	}else{
		DialogMsg msg( this ) ;

		msg.ShowUIOK( tr( "ERROR" ),m ) ;
		m_ui->lineEditKey->clear() ;
		this->enableAll() ;
		m_ui->lineEditKey->setFocus() ;
	}
}

void keyDialog::getPassWord()
{
	if( m_key.isEmpty() ){
		DialogMsg msg( this ) ;
		msg.ShowUIOK( tr( "ERROR" ),tr( "the volume does not appear to have an entry in the wallet" ) ) ;
		this->enableAll() ;
		if( m_ui->cbKeyType->currentIndex() != keyDialog::Key ){
			m_ui->lineEditKey->setEnabled( false ) ;
		}
	}else{
		this->openVolume() ;
	}

	m_wallet->deleteLater() ;
}

void keyDialog::walletIsOpen( bool opened )
{
	if( opened ){

		auto _getKey = [&](){
			m_key = utility::getKeyFromWallet( m_wallet,m_path ) ;
		} ;

		Task * t = new Task() ;
		connect( t,SIGNAL( done() ),this,SLOT( getPassWord() ) ) ;
		t->setFunction( _getKey ) ;
		t->start( Task::getKey ) ;
	}else{
		_internalPassWord.clear() ;
		this->enableAll() ;
		m_wallet->deleteLater() ;
	}
}

void keyDialog::getPassWord( QString password )
{
	_internalPassWord = password ;
}

void keyDialog::pbOpen()
{
	this->disableAll() ;
	m_key.clear() ;
	if( m_ui->cbKeyType->currentIndex() == keyDialog::plugin ){
		QString r = m_ui->lineEditKey->text() ;
		if( r == tr( KWALLET ) ){
			m_wallet = LxQt::Wallet::getWalletBackend( LxQt::Wallet::kwalletBackEnd ) ;
			m_wallet->setInterfaceObject( this ) ;
			m_wallet->open( m_wallet->localDefaultWalletName(),utility::applicationName() ) ;
		}else if( r == tr( INTERNAL_WALLET ) ){
			QString walletName = utility::walletName() ;
			QString appName    = utility::applicationName() ;
			if( LxQt::Wallet::walletExists( LxQt::Wallet::internalBackEnd,walletName,appName ) ){
				m_wallet = LxQt::Wallet::getWalletBackend( LxQt::Wallet::internalBackEnd ) ;
				m_wallet->setInterfaceObject( this ) ;
				QObject * obj = m_wallet->qObject() ;
				connect( obj,SIGNAL( getPassWord( QString ) ),this,SLOT( getPassWord( QString ) ) ) ;
				m_wallet->open( walletName,appName,_internalPassWord,"zuluMount" ) ;
			}else{
				DialogMsg msg( this ) ;
				msg.ShowUIOK( tr( "ERROR!" ),tr( "internal wallet is not configured" ) ) ;
				this->enableAll() ;
			}
		}else if( r == tr( GNOME_WALLET ) ){
			m_wallet = LxQt::Wallet::getWalletBackend( LxQt::Wallet::secretServiceBackEnd ) ;
			m_wallet->setInterfaceObject( this ) ;
			m_wallet->open( utility::walletName(),utility::applicationName() ) ;
		}else{
			this->openVolume() ;
		}
	}else{
		this->openVolume() ;
	}
}

void keyDialog::openVolume()
{
	int keyType = m_ui->cbKeyType->currentIndex() ;

	if( m_ui->lineEditKey->text().isEmpty() ){
		if( keyType == keyDialog::Key ){
			;
		}else if( keyType == keyDialog::plugin ){
			DialogMsg msg( this ) ;
			msg.ShowUIOK( tr( "ERROR" ),tr( "plug in name field is empty" ) ) ;
			m_ui->lineEditKey->setFocus() ;
			return this->enableAll() ;
		}else if( keyType == keyDialog::keyfile ){
			DialogMsg msg( this ) ;
			msg.ShowUIOK( tr( "ERROR" ),tr( "keyfile field is empty" ) ) ;
			m_ui->lineEditKey->setFocus() ;
			return this->enableAll() ;
		}
	}

	QString test_name = m_ui->lineEditMountPoint->text() ;
	if( test_name.contains( QString( "/" ) ) ){
		DialogMsg msg( this ) ;
		msg.ShowUIOK( tr( "ERROR" ),tr( "\"/\" character is not allowed in the mount name field" ) ) ;
		m_ui->lineEditKey->setFocus() ;
		return this->enableAll() ;
	}

	QString m ;
	if( keyType == keyDialog::Key ){
		QString addr = utility::keyPath() ;
		m = QString( "-f ") + addr ;
		Task * t = new Task() ;
		t->setKey( m_ui->lineEditKey->text() ) ;
		t->setKeyPath( addr ) ;
		t->start( Task::sendKey ) ;

	}else if( keyType == keyDialog::keyfile ){
		m = QString( "-f ") + m_ui->lineEditKey->text().replace( "\"","\"\"\"" ) ;
	}else if( keyType == keyDialog::plugin ){
		if( m_key.isEmpty() ){
			m = QString( "-G ") + m_ui->lineEditKey->text().replace( "\"","\"\"\"" ) ;
		}else{
			QString addr = utility::keyPath() ;
			m = QString( "-f ") + addr ;
			Task * t = new Task() ;
			t->setKey( m_key ) ;
			t->setKeyPath( addr ) ;
			t->start( Task::sendKey ) ;
		}
	}

	Task * t = new Task() ;

	t->setDevice( m_path ) ;

	if( m_options.isEmpty() ){
		if( m_ui->checkBoxOpenReadOnly->isChecked() ){
			t->setMode( QString( "ro" ) ) ;
		}else{
			t->setMode( QString( "rw" ) ) ;
		}
	}else{
		if( m_ui->checkBoxOpenReadOnly->isChecked() ){
			t->setMode( QString( "ro -Y %1" ).arg( m_options ) ) ;
		}else{
			t->setMode( QString( "rw -Y %1" ).arg( m_options ) ) ;
		}
	}
	if( m_deviceOffSet.isEmpty() ){
		t->setKeySource( m ) ;
	}else{
		t->setKeySource( m + m_deviceOffSet ) ;
	}

	m_point = m_ui->lineEditMountPoint->text() ;

	t->setMountPoint( m_point ) ;
	t->setMakeMountPointPublic( m_ui->checkBoxShareMountPoint->isChecked() ) ;

	m_working = true ;

	connect( t,SIGNAL( signalMountComplete( int,QString ) ),this,SLOT( slotMountComplete( int,QString ) ) ) ;
	t->start( Task::CryptoOpen ) ;
}

void keyDialog::cbActicated( int e )
{
	switch( e ){
		case keyDialog::Key     : return this->key() ;
		case keyDialog::keyfile : return this->keyFile() ;
		case keyDialog::plugin  : return this->plugIn() ;
	}
}


void keyDialog::plugIn()
{
	m_ui->pbkeyOption->setIcon( QIcon( QString( ":/module.png" ) ) ) ;
	m_ui->lineEditKey->setEchoMode( QLineEdit::Normal ) ;
	m_ui->label->setText( tr( "plugin name" ) ) ;
	m_ui->pbkeyOption->setEnabled( true ) ;
	m_ui->lineEditKey->setEnabled( false ) ;
	m_ui->lineEditKey->setText( INTERNAL_WALLET ) ;
}

void keyDialog::key()
{
	m_ui->pbkeyOption->setIcon( QIcon( QString( ":/passphrase.png" ) ) ) ;
	m_ui->pbkeyOption->setEnabled( false ) ;
	m_ui->label->setText( tr( "key" ) ) ;
	m_ui->lineEditKey->setEchoMode( QLineEdit::Password ) ;
	m_ui->lineEditKey->clear() ;
	m_ui->lineEditKey->setEnabled( true ) ;
}

void keyDialog::keyFile()
{
	m_ui->pbkeyOption->setIcon( QIcon( QString( ":/keyfile.png" ) ) ) ;
	m_ui->lineEditKey->setEchoMode( QLineEdit::Normal ) ;
	m_ui->label->setText( tr( "keyfile path" ) ) ;
	m_ui->pbkeyOption->setEnabled( true ) ;
	m_ui->lineEditKey->clear() ;
	m_ui->lineEditKey->setEnabled( false ) ;
}

void keyDialog::pbCancel()
{
	this->HideUI() ;
	emit cancel() ;
}

void keyDialog::ShowUI()
{
	this->show() ;
}

void keyDialog::HideUI()
{
	if( !m_working ){
		this->hide() ;
		this->deleteLater() ;
	}
}

keyDialog::~keyDialog()
{
	m_menu->deleteLater() ;
	delete m_ui ;
}
