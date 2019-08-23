# Setup all public headers, this needs to be kept in
# sync with truly used headers.
INSTALL_HEADERS = \
../librssg/core/feeddownloader.h \
../librssg/core/feedsmodel.h \
../librssg/core/feedsproxymodel.h \
../librssg/core/message.h \
../librssg/core/messagesmodel.h \
../librssg/core/messagesmodelcache.h \
../librssg/core/messagesmodelsqllayer.h \
../librssg/core/messagesproxymodel.h \
../librssg/definitions/definitions.h \
../librssg/dynamic-shortcuts/dynamicshortcuts.h \
../librssg/dynamic-shortcuts/dynamicshortcutswidget.h \
../librssg/dynamic-shortcuts/shortcutbutton.h \
../librssg/dynamic-shortcuts/shortcutcatcher.h \
../librssg/exceptions/applicationexception.h \
../librssg/exceptions/ioexception.h \
../librssg/gui/baselineedit.h \
../librssg/gui/basetoolbar.h \
../librssg/gui/colorlabel.h \
../librssg/gui/comboboxwithstatus.h \
../librssg/gui/dialogs/formabout.h \
../librssg/gui/dialogs/formaddaccount.h \
../librssg/gui/dialogs/formbackupdatabasesettings.h \
../librssg/gui/dialogs/formdatabasecleanup.h \
../librssg/gui/dialogs/formmain.h \
../librssg/gui/dialogs/formrestoredatabasesettings.h \
../librssg/gui/dialogs/formsettings.h \
../librssg/gui/dialogs/formupdate.h \
../librssg/gui/dialogs/oauthlogin.h \
../librssg/gui/discoverfeedsbutton.h \
../librssg/gui/edittableview.h \
../librssg/gui/feedmessageviewer.h \
../librssg/gui/feedstoolbar.h \
../librssg/gui/feedsview.h \
../librssg/gui/guiutilities.h \
../librssg/gui/labelwithstatus.h \
../librssg/gui/lineeditwithstatus.h \
../librssg/gui/locationlineedit.h \
../librssg/gui/messagebox.h \
../librssg/gui/messagepreviewer.h \
../librssg/gui/messagessearchlineedit.h \
../librssg/gui/messagestoolbar.h \
../librssg/gui/messagesview.h \
../librssg/gui/messagetextbrowser.h \
../librssg/gui/newspaperpreviewer.h \
../librssg/gui/plaintoolbutton.h \
../librssg/gui/searchtextwidget.h \
../librssg/gui/settings/settingsbrowsermail.h \
../librssg/gui/settings/settingsdatabase.h \
../librssg/gui/settings/settingsdownloads.h \
../librssg/gui/settings/settingsfeedsmessages.h \
../librssg/gui/settings/settingsgeneral.h \
../librssg/gui/settings/settingsgui.h \
../librssg/gui/settings/settingslocalization.h \
../librssg/gui/settings/settingspanel.h \
../librssg/gui/settings/settingsshortcuts.h \
../librssg/gui/squeezelabel.h \
../librssg/gui/statusbar.h \
../librssg/gui/styleditemdelegatewithoutfocus.h \
../librssg/gui/systemtrayicon.h \
../librssg/gui/tabbar.h \
../librssg/gui/tabcontent.h \
../librssg/gui/tabwidget.h \
../librssg/gui/timespinbox.h \
../librssg/gui/toolbareditor.h \
../librssg/gui/treeviewcolumnsmenu.h \
../librssg/gui/treewidget.h \
../librssg/gui/webbrowser.h \
../librssg/gui/webviewer.h \
../librssg/gui/widgetwithstatus.h \
../librssg/miscellaneous/application.h \
../librssg/miscellaneous/autosaver.h \
../librssg/miscellaneous/databasecleaner.h \
../librssg/miscellaneous/databasefactory.h \
../librssg/miscellaneous/databasequeries.h \
../librssg/miscellaneous/debugging.h \
../librssg/miscellaneous/externaltool.h \
../librssg/miscellaneous/feedreader.h \
../librssg/miscellaneous/iconfactory.h \
../librssg/miscellaneous/iofactory.h \
../librssg/miscellaneous/localization.h \
../librssg/miscellaneous/mutex.h \
../librssg/miscellaneous/regexfactory.h \
../librssg/miscellaneous/settings.h \
../librssg/miscellaneous/settingsproperties.h \
../librssg/miscellaneous/simplecrypt/simplecrypt.h \
../librssg/miscellaneous/skinfactory.h \
../librssg/miscellaneous/systemfactory.h \
../librssg/miscellaneous/textfactory.h \
../librssg/network-web/adblock/adblockaddsubscriptiondialog.h \
../librssg/network-web/adblock/adblockdialog.h \
../librssg/network-web/adblock/adblockicon.h \
../librssg/network-web/adblock/adblockmanager.h \
../librssg/network-web/adblock/adblockmatcher.h \
../librssg/network-web/adblock/adblockrule.h \
../librssg/network-web/adblock/adblocksearchtree.h \
../librssg/network-web/adblock/adblocksubscription.h \
../librssg/network-web/adblock/adblocktreewidget.h \
../librssg/network-web/adblock/adblockurlinterceptor.h \
../librssg/network-web/basenetworkaccessmanager.h \
../librssg/network-web/downloader.h \
../librssg/network-web/downloadmanager.h \
../librssg/network-web/googlesuggest.h \
../librssg/network-web/httpresponse.h \
../librssg/network-web/networkfactory.h \
../librssg/network-web/networkurlinterceptor.h \
../librssg/network-web/oauth2service.h \
../librssg/network-web/oauthhttphandler.h \
../librssg/network-web/rssguardschemehandler.h \
../librssg/network-web/silentnetworkaccessmanager.h \
../librssg/network-web/urlinterceptor.h \
../librssg/network-web/webfactory.h \
../librssg/network-web/webpage.h \
../librssg/qtsingleapplication/qtlocalpeer.h \
../librssg/qtsingleapplication/qtlockedfile.h \
../librssg/qtsingleapplication/qtsingleapplication.h \
../librssg/qtsingleapplication/qtsinglecoreapplication.h \
../librssg/services/abstract/accountcheckmodel.h \
../librssg/services/abstract/cacheforserviceroot.h \
../librssg/services/abstract/category.h \
../librssg/services/abstract/feed.h \
../librssg/services/abstract/gui/formfeeddetails.h \
../librssg/services/abstract/recyclebin.h \
../librssg/services/abstract/rootitem.h \
../librssg/services/abstract/serviceentrypoint.h \
../librssg/services/abstract/serviceroot.h \
../librssg/services/gmail/definitions.h \
../librssg/services/gmail/gmailentrypoint.h \
../librssg/services/gmail/gmailfeed.h \
../librssg/services/gmail/gmailserviceroot.h \
../librssg/services/gmail/gui/formaddeditemail.h \
../librssg/services/gmail/gui/formdownloadattachment.h \
../librssg/services/gmail/gui/formeditgmailaccount.h \
../librssg/services/gmail/network/gmailnetworkfactory.h \
../librssg/services/inoreader/definitions.h \
../librssg/services/inoreader/gui/formeditinoreaderaccount.h \
../librssg/services/inoreader/inoreaderentrypoint.h \
../librssg/services/inoreader/inoreaderfeed.h \
../librssg/services/inoreader/inoreaderserviceroot.h \
../librssg/services/inoreader/network/inoreadernetworkfactory.h \
../librssg/services/owncloud/definitions.h \
../librssg/services/owncloud/gui/formeditowncloudaccount.h \
../librssg/services/owncloud/gui/formowncloudfeeddetails.h \
../librssg/services/owncloud/network/owncloudnetworkfactory.h \
../librssg/services/owncloud/owncloudfeed.h \
../librssg/services/owncloud/owncloudserviceentrypoint.h \
../librssg/services/owncloud/owncloudserviceroot.h \
../librssg/services/standard/atomparser.h \
../librssg/services/standard/feedparser.h \
../librssg/services/standard/gui/formstandardcategorydetails.h \
../librssg/services/standard/gui/formstandardfeeddetails.h \
../librssg/services/standard/gui/formstandardimportexport.h \
../librssg/services/standard/rdfparser.h \
../librssg/services/standard/rssparser.h \
../librssg/services/standard/standardcategory.h \
../librssg/services/standard/standardfeed.h \
../librssg/services/standard/standardfeedsimportexportmodel.h \
../librssg/services/standard/standardserviceentrypoint.h \
../librssg/services/standard/standardserviceroot.h \
../librssg/services/tt-rss/definitions.h \
../librssg/services/tt-rss/gui/formeditttrssaccount.h \
../librssg/services/tt-rss/gui/formttrssfeeddetails.h \
../librssg/services/tt-rss/network/ttrssnetworkfactory.h \
../librssg/services/tt-rss/ttrssfeed.h \
../librssg/services/tt-rss/ttrssserviceentrypoint.h \
../librssg/services/tt-rss/ttrssserviceroot.h

# Install all files on Windows.
win32 {
  target.path = $$PREFIX
  
  lib.files = $$OUT_PWD/../librssg/librssg.dll $$OUT_PWD/../librssg/librssg.lib
  lib.path = $$PREFIX
  lib.CONFIG = no_check_exist

  qt_dlls_root.files = ../../resources/binaries/windows/qt5-msvc2017/*.*
  qt_dlls_root.path = $$quote($$PREFIX/)

  qt_dlls_plugins.files = ../../resources/binaries/windows/qt5-msvc2017/*
  qt_dlls_plugins.path = $$quote($$PREFIX/)

  INSTALLS += target lib qt_dlls_root qt_dlls_plugins

  equals(USE_WEBENGINE, true) {
    # Copy extra resource files for QtWebEngine.
    qtwebengine_dlls.files = ../../resources/binaries/windows/qt5-msvc2017-webengine/*
    qtwebengine_dlls.path = $$quote($$PREFIX/)

    qtwebengine.files = ../../resources/binaries/windows/qt5-msvc2017-webengine/*.*
    qtwebengine.path = $$quote($$PREFIX/)

    INSTALLS += qtwebengine_dlls qtwebengine
  }
  
  INSTALL_HEADERS_PREFIX = $$quote($$PREFIX/include/librssg/)
}

# Install all files on Linux.
unix:!mac:!android {
  target.path = $$PREFIX/bin

  desktop_file.files = ../../resources/desktop/$${APP_REVERSE_NAME}.desktop
  desktop_file.path = $$quote($$PREFIX/share/applications/)

  appdata.files = ../../resources/desktop/$${APP_REVERSE_NAME}.appdata.xml
  appdata.path = $$quote($$PREFIX/share/metainfo/)

  lib.files = $$OUT_PWD/../librssg/librssg.so
  lib.path = $$quote($$PREFIX/lib/)
  lib.CONFIG = no_check_exist

  desktop_icon.files = ../../resources/graphics/$${TARGET}.png
  desktop_icon.path = $$quote($$PREFIX/share/icons/hicolor/512x512/apps/)

  INSTALLS += target lib desktop_file desktop_icon appdata
  
  INSTALL_HEADERS_PREFIX = $$quote($$PREFIX/include/librssg/)
}

mac {
  IDENTIFIER = $$APP_REVERSE_NAME
  CONFIG -= app_bundle
  ICON = ../../resources/macosx/$${TARGET}.icns
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
  LIBS += -framework AppKit

  QMAKE_POST_LINK += $$system(install_name_tool -change "librssg.dylib" "@executable_path/librssg.dylib" $$OUT_PWD/rssguard)

  target.path = $$quote($$PREFIX/Contents/MacOS/)

  lib.files = $$OUT_PWD/../librssg/librssg.dylib
  lib.path = $$quote($$PREFIX/Contents/MacOS/)
  lib.CONFIG = no_check_exist

  # Install app icon.
  icns_icon.files = ../../resources/macosx/$${TARGET}.icns
  icns_icon.path = $$quote($$PREFIX/Contents/Resources/)

  # Install Info.plist.
  info_plist.files = ../../resources/macosx/Info.plist.in
  info_plist.path  = $$quote($$PREFIX/Contents/)

  # Process the just installed Info.plist.
  info_plist2.extra = @sed -e "s,@EXECUTABLE@,$$TARGET,g" -e "s,@SHORT_VERSION@,$$APP_VERSION,g" -e "s,@APP_NAME@,\"$$APP_NAME\",g" -e "s,@ICON@,$$basename(ICON),g"  -e "s,@TYPEINFO@,"????",g" $$shell_quote($$PREFIX/Contents/Info.plist.in) > $$shell_quote($$PREFIX/Contents/Info.plist) && \
                      rm -f $$shell_quote($$PREFIX/Contents/Info.plist.in)
  info_plist2.path = $$quote($$PREFIX/Contents/)

  # Install PkgInfo
  pkginfo.extra = @printf "APPL????" > $$shell_quote($$PREFIX/Contents/PkgInfo)
  pkginfo.path = $$quote($$PREFIX/Contents/)

  INSTALLS += target lib icns_icon info_plist info_plist2 pkginfo
  
  INSTALL_HEADERS_PREFIX = $$quote($$PREFIX/Contents/Resources/Include/libtextosaurus/)
}

# Create install step for each folder of public headers.
for(header, INSTALL_HEADERS) {
  path = $${INSTALL_HEADERS_PREFIX}/$${dirname(header)}

  message($$MSG_PREFIX: Adding header \"$$header\" to \"make install\" step.)

  eval(headers_$${dirname(header)}.files += $$header)
  eval(headers_$${dirname(header)}.path = $$path)
  eval(INSTALLS *= headers_$${dirname(header)})
}
