#include <QDomDocument>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <stdlib.h>

#include "ModuleImport.h"
#include "Song.h"
#include "Engine.h"
#include "Note.h"
#include "Instrument.h"
#include "InstrumentTrack.h"
#include "Note.h"
#include "Pattern.h"
#include "Track.h"
#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "Instrument.h"

#include "plugin_export.h"

#include "libopenmpt/libopenmpt.hpp"

#define MAX_LAYERS 4

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT moduleimport_plugin_descriptor =
{
	STRINGIFY(PLUGIN_NAME),
	"Module Import",
	QT_TRANSLATE_NOOP( "PluginBrowser",
				"Filter for importing module files into LMMS" ),
	"",
	0x0100,
	Plugin::ImportFilter,
	NULL,
	NULL,
	NULL
} ;

}

QString filename;

ModuleImport::ModuleImport(const QString &_file) :
	ImportFilter(_file, &moduleimport_plugin_descriptor)
{
	filename = _file;
}

ModuleImport::~ModuleImport()
{
}

bool ModuleImport::readSong(TrackContainer* tc)
{
	printf("get_library_version: %d\n", openmpt::get_library_version());
	printf("get_core_version: %d\n", openmpt::get_core_version());

	QByteArray data = readAllData();
	openmpt::module module(data.data(), data.size());

	printf("channels: %d\n", module.get_num_channels());
	printf("samples: %d\n", module.get_num_samples());
	printf("instruments: %d\n", module.get_num_instruments());

	Song *s = Engine::getSong();

	InstrumentTrack *it = dynamic_cast<InstrumentTrack *>(Track::create(Track::InstrumentTrack, tc));
	it->loadInstrument("tripleoscillator" );
	it->setName("Track #1");

	Pattern* p = dynamic_cast<Pattern*>(it->createTCO(0));

	for (int i = 0; i < 12 * 8; ++i) {
		Note n(10, i * 10, i);
		p->addNote((const Note&) n, false);
	}

	return true;
}

bool ModuleImport::tryImport(TrackContainer* tc)
{
	if (openFile() == false)
	{
		return false;
	}
	return readSong(tc);
}

extern "C"
{

PLUGIN_EXPORT Plugin * lmms_plugin_main(Model *, void *_data)
{
	return new ModuleImport(QString::fromUtf8(static_cast<const char *>(_data)));
}

}

