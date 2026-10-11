#include "theme_manager.h"

static QPalette themePalette(bool dark) {
 QPalette p;
 const QColor window(dark?"#353535":"#f5f7fa"),base(dark?"#2a2a2a":"#ffffff"),
  text(dark?"#ffffff":"#202b38"),button(dark?"#353535":"#edf1f5");
 for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled}) {
  const bool disabled=group==QPalette::Disabled;
  p.setColor(group,QPalette::Window,window);p.setColor(group,QPalette::Base,base);
  p.setColor(group,QPalette::AlternateBase,QColor(dark?"#424242":"#f0f3f7"));
  p.setColor(group,QPalette::Button,button);
  for(auto role:{QPalette::WindowText,QPalette::Text,QPalette::ButtonText})p.setColor(group,role,disabled?QColor(dark?"#7f7f7f":"#929ba6"):text);
  p.setColor(group,QPalette::Highlight,QColor(disabled?(dark?"#505050":"#dce3ec"):"#2a82da"));
  p.setColor(group,QPalette::HighlightedText,disabled?QColor(dark?"#7f7f7f":"#737e8c"):QColor("#ffffff"));
  p.setColor(group,QPalette::ToolTipBase,base);p.setColor(group,QPalette::ToolTipText,text);
  p.setColor(group,QPalette::Link,QColor("#2a82da"));
  p.setColor(group,QPalette::Light,QColor(dark?"#707070":"#ffffff"));
  p.setColor(group,QPalette::Mid,QColor(dark?"#404040":"#ccd4df"));
  p.setColor(group,QPalette::Dark,QColor(dark?"#232323":"#aab6c5"));
  p.setColor(group,QPalette::Shadow,QColor(dark?"#141414":"#d3dae4"));
  p.setColor(group,QPalette::PlaceholderText,QColor(dark?"#9a9a9a":"#8893a1"));
 }return p;
}

// Recolor monochrome glyphs; logos, device art and colored icons stay intact.
static QIcon themedIcon(const QIcon &source,bool dark) {
 if(source.isNull())return source;
 QIcon result;bool changed=false;
 auto sizes=source.availableSizes();if(sizes.isEmpty())sizes<<QSize(24,24);
 for(auto size:sizes){QImage im=source.pixmap(size).toImage().convertToFormat(QImage::Format_ARGB32);
  int visible=0,white=0,black=0,colored=0;
  for(int y=0;y<im.height();++y)for(int x=0;x<im.width();++x){QColor c=im.pixelColor(x,y);if(c.alpha()<32)continue;++visible;if(qMax(c.red(),qMax(c.green(),c.blue()))-qMin(c.red(),qMin(c.green(),c.blue()))>25)++colored;if(c.red()>200&&c.green()>200&&c.blue()>200)++white;if(c.red()<70&&c.green()<70&&c.blue()<70)++black;}
  if(visible&&colored==0&&(white>visible*0.9||black>visible*0.9)){changed=true;for(int y=0;y<im.height();++y)for(int x=0;x<im.width();++x){QColor c(dark?"#ffffff":"#344253");c.setAlpha(im.pixelColor(x,y).alpha());im.setPixelColor(x,y,c);}}
  result.addPixmap(QPixmap::fromImage(im));
 }return changed?result:source;
}

ThemeManager::ThemeManager(QWidget *w):QObject(w),root(w) {
 darkToggle=root->findChild<QCheckBox*>("chk_login_darkmode");
 selector=root->findChild<QComboBox*>("themeSelector");
 for(auto child:root->findChildren<QWidget*>()){
  originalStyles[child]=child->styleSheet();
  if(auto b=qobject_cast<QAbstractButton*>(child))originalIcons[b]=b->icon();
  if(auto t=qobject_cast<QTextEdit*>(child))originalHtml[t]=t->toHtml();
 }
 if(darkToggle)connect(darkToggle,&QCheckBox::toggled,this,[this](bool enabled){setDark(enabled);});
 if(selector)connect(selector,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int index){setDark(index==1);});
 auto hotkey=new QShortcut(QKeySequence("Ctrl+Shift+T"),root);
 connect(hotkey,&QShortcut::activated,this,[this]{setDark(!dark);});
 QSettings settings("TeraToolUI","DesignerReplica");
 QString requested=settings.value("appearance/theme","dark").toString();
 const auto args=qApp->arguments();int index=args.indexOf("--theme");if(index>=0)requested=args.value(index+1).toLower();
 if(args.contains("--verify-navigation")||args.contains("--verify-theme"))requested=index>=0?args.value(index+1):"dark";
 setDark(requested!="light",false);
}

void ThemeManager::synchronizeControls() {
 if(darkToggle){QSignalBlocker block(darkToggle);darkToggle->setChecked(dark);}
 if(selector){QSignalBlocker block(selector);selector->setCurrentIndex(dark?1:0);}
 root->setProperty("darkTheme",dark);
}

void ThemeManager::setDark(bool enabled,bool persist) {
 // Unpolish the old stylesheet before changing palettes. Otherwise Qt restores
 // the previous saved palette on repolish, leaving hidden pages in the old theme.
 qApp->setStyleSheet(QString());
 const auto widgets=root->findChildren<QWidget*>();
 for(auto w:widgets)w->setStyleSheet(QString());
 dark=enabled;const auto palette=themePalette(dark);qApp->setPalette(palette);root->setPalette(palette);
 for(auto w:root->findChildren<QWidget*>()){
  w->setPalette(palette);QString css=originalStyles.value(w);
  if(css.contains("QTableWidget::item:hover"))css.clear();
  if(!dark){css.replace("#2a2a2a","#edf1f5",Qt::CaseInsensitive);css.replace("#262626","#ccd4df",Qt::CaseInsensitive);}
  w->setStyleSheet(css);
 }
 QFile file(dark?":/themes/dark.qss":":/themes/light.qss");file.open(QIODevice::ReadOnly);qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
 // Stylesheet repolishing can restore a cached child palette, even after the
 // application stylesheet was removed. Apply the new palette after polish too.
 root->ensurePolished();root->setPalette(palette);
 for(auto w:root->findChildren<QWidget*>()){w->ensurePolished();w->setPalette(palette);}
 for(auto i=originalIcons.begin();i!=originalIcons.end();++i){const char *key=dark?"darkThemeIcon":"lightThemeIcon";QVariant cached=i.key()->property(key);if(!cached.isValid()){cached=QVariant::fromValue(themedIcon(i.value(),dark));i.key()->setProperty(key,cached);}i.key()->setIcon(cached.value<QIcon>());}
 for(auto i=originalHtml.begin();i!=originalHtml.end();++i){QString html=i.value();if(!dark){html.replace("#ffffff","#202b38",Qt::CaseInsensitive);html.replace("#353535","#f5f7fa",Qt::CaseInsensitive);html.replace("#2a2a2a","#ffffff",Qt::CaseInsensitive);}i.key()->setHtml(html);}
 synchronizeControls();root->update();
 if(persist&&!qApp->arguments().contains("--verify-theme")&&!qApp->arguments().contains("--verify-navigation")){QSettings settings("TeraToolUI","DesignerReplica");settings.setValue("appearance/theme",dark?"dark":"light");}
}
