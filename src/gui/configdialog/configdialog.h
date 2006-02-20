/****************************************************************
 *  $Id$ 
 * 
 *  Vidalia is distributed under the following license:
 *
 *  Copyright (C) 2006,  Matt Edman, Justin Hipple
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef _CONFIGDIALOG_H
#define _CONFIGDIALOG_H

#include <QFileDialog>

#include "ui_configdialog.h"

#include "../../control/torcontrol.h"
#include "../../config/vidaliasettings.h"
#include "../../config/serversettings.h"


class ConfigDialog : public QDialog
{
  Q_OBJECT

public:
  /** Default Constructor */
  ConfigDialog(TorControl *torControl, QWidget *parent = 0);
  /** Default Destructor */
  ~ConfigDialog();

public slots:
  /** Called when user selects an item from the settings page list */
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);
  /** Called when this dialog is to be displayed */
  void show();
  
private slots:
  /** Called when user clicks "Cancel" */
  void cancelChanges();
  /** Called when user clicks "Save Settings" */
  void saveChanges();
  /** Called when user clicks "Browse" to choose location of Tor executable */
  void browseTorPath();
  /** Called when user clicks "Browse" to choose location of Tor config file */
  void browseTorConfig();
  /** Called when the user clicks "Get Address" to guess our local IP */
  void getServerAddress();

private:
  /** Connects actions to events */
  void createActions();
  
  /** Loads the current configuration settings */
  void loadSettings();
  void loadGeneralSettings();
  void loadServerSettings();
  void loadAdvancedSettings();

  /** Saves the current configuration settings */
  void saveGeneralSettings();
  bool saveServerSettings(QString *errmsg = 0);
  void saveAdvancedSettings();

  /** Toggles display of server config frame */
  void showServerConfig(bool show);
  /** Attempts to find the server's public IP address */
  void getServerPublicIP();

  /** A TorControl object used to talk to Tor. */
  TorControl* _torControl;
  /** A VidaliaSettings object that handles getting/saving settings */
  VidaliaSettings* _settings;
  /** A ServerSettings object used to get and set information about how a
   * local Tor server is configured. */
  ServerSettings*  _serverSettings;
  
  /** Qt Designer generated object */
  Ui::ConfigDialog ui;
};

#endif

