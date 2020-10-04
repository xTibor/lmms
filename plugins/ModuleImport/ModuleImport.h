#ifndef _MODULE_IMPORT_H
#define _MODULE_IMPORT_H

#include <QString>
#include <QPair>
#include <QVector>

#include "ImportFilter.h"


class ModuleImport : public ImportFilter
{
public:
	ModuleImport( const QString & _file );
	bool readSong(TrackContainer* tc);

	virtual ~ModuleImport();

	virtual PluginView * instantiateView( QWidget * )
	{
		return( NULL );
	}
private:
	virtual bool tryImport( TrackContainer* tc );
};
#endif

