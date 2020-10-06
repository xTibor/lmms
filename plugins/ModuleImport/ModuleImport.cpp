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

	//Song *s = Engine::getSong();

	QVector<InstrumentTrack *> tracks;
	std::vector<std::string> instrument_names;
	int instrument_count;

	if (module.get_num_instruments() > 0) {
		instrument_names = module.get_instrument_names();
		instrument_count = module.get_num_instruments();
	} else {
		instrument_names = module.get_sample_names();
		instrument_count = module.get_num_samples();
	}

	for (int i = 1; i <= instrument_count; ++i) {
		InstrumentTrack *track = dynamic_cast<InstrumentTrack *>(Track::create(Track::InstrumentTrack, tc));
		track->loadInstrument("tripleoscillator");
		track->setName(QString::fromStdString(instrument_names[i - 1]));

		tracks.append(track);
	}

	for (int i = 1; i <= instrument_count; ++i) {
		printf("Instrument %d\n", i);
		int pattern_position = 0;

		for (int mod_order = 0; mod_order < module.get_num_orders(); ++mod_order) {
			auto mod_pattern = module.get_order_pattern(mod_order);
			auto mod_pattern_length = module.get_pattern_num_rows(mod_pattern);

			Pattern* pattern = dynamic_cast<Pattern*>(tracks[i - 1]->createTCO(0));
			pattern->movePosition(pattern_position);
			pattern_position += mod_pattern_length * DefaultTicksPerBar / 16;

			for (int mod_channel = 0; mod_channel < module.get_num_channels(); ++mod_channel) {
				int mod_row = 0;

				#define CURRENT_NOTE() (module.get_pattern_row_channel_command(mod_pattern, mod_row, mod_channel, openmpt::module::command_note))
				#define CURRENT_INSTRUMENT() (module.get_pattern_row_channel_command(mod_pattern, mod_row, mod_channel, openmpt::module::command_instrument))
				#define CURRENT_VOLUME() (module.get_pattern_row_channel_command(mod_pattern, mod_row, mod_channel, openmpt::module::command_volume))

				while (mod_row < mod_pattern_length) {
					// Seek to the first note
					while (
						(mod_row < mod_pattern_length) &&
						(
							(CURRENT_NOTE() < ModCommandNote::NOTE_MIN) ||
							(CURRENT_NOTE() > ModCommandNote::NOTE_MAX)
						)
					)
					{
						++mod_row;
					}

					// Break if no note found
					if (mod_row == mod_pattern_length) {
						break;
					}

					int note_position = mod_row;
					int note_pitch = CURRENT_NOTE();
					int note_instrument = CURRENT_INSTRUMENT();
					int note_volume = CURRENT_VOLUME();
					int note_length = 1;
					++mod_row;

					if (note_volume == 0)
						note_volume = 31;

					while (
						(mod_row < mod_pattern_length) &&
						!(
							(CURRENT_NOTE() == ModCommandNote::NOTE_KEYOFF) ||
							(CURRENT_NOTE() == ModCommandNote::NOTE_NOTECUT) ||
							(CURRENT_NOTE() == ModCommandNote::NOTE_FADE) ||
							(
								(CURRENT_NOTE() >= ModCommandNote::NOTE_MIN) &&
								(CURRENT_NOTE() <= ModCommandNote::NOTE_MAX)
							)
						)
					)
					{
						++note_length;
						++mod_row;
					}

					//printf("pitch: %d, position: %d, length: %d\n", note_pitch, note_position, note_length);

					if ((note_length > 0) && (note_instrument == i)) {
						Note n(DefaultTicksPerBar / 16 * note_length, DefaultTicksPerBar / 16 * note_position, note_pitch, note_volume * 4);
						pattern->addNote((const Note&) n, false);
					}

				}

				#undef CURRENT_NOTE
				#undef CURRENT_INSTRUMENT
				#undef CURRENT_VOLUME
			}
		}
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

