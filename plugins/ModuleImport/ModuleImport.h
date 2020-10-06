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
	// libopenmpt doesn't export these enums. Taken from:
	// https://github.com/OpenMPT/openmpt/blob/f56ac1317aa81b66a35f9616fb90c28ba25c8cd4/soundlib/modcommand.h#L23-L36
	enum ModCommandNote
	{
		NOTE_NONE        = 0,
		NOTE_MIN         = 1,
		NOTE_MAX         = 120,
		NOTE_MIDDLEC     = (5 * 12 + NOTE_MIN),
		NOTE_KEYOFF      = 0xFF,
		NOTE_NOTECUT     = 0xFE,
		NOTE_FADE        = 0xFD,
		NOTE_PC          = 0xFC,
		NOTE_PCS         = 0xFB,
		NOTE_MIN_SPECIAL = NOTE_PCS,
		NOTE_MAX_SPECIAL = NOTE_KEYOFF,
	};

	virtual bool tryImport( TrackContainer* tc );
};
#endif

