#include <lrtypes.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Box.H>
#include <set>
#include <stdio.h>

struct quad {
	u8 bytes[8*8*4];
};

u32 bads;

static void mess(Fl_RGB_Image *img) {

	const u8 depth = img->d();

	if (img->w() % 8 || img->h() % 8 || img->d() < 3) {
		fl_message("Error, image not divisible by 8");
		abort();
	}

	const u32 numw = img->w() / 8;
	const u32 numh = img->h() / 8;

	quad q;
	u8 * const ptr = (u8 *) img->array;

	// Read them
	const u32 pixelrow = img->w() * depth;
	u32 x, y, i;
	i = 0;
	for (y = 0; y < numh; y++) {
		for (x = 0; x < numw; x++, i++) {
			u32 r, c;
			for (r = 0; r < 8; r++) {
				// Copy row r
				memcpy(&q.bytes[r * 8 * depth],
					&ptr[(y * 8 + r) * pixelrow +
						x * 8 * depth],
					8 * depth);
			}

			// Check
			std::set<u32> pixels;
			for (r = 0; r < 64; r++) {
				u32 p = 0;
				memcpy(&p, &q.bytes[r * depth], depth);
				pixels.insert(p);
			}

			if (pixels.size() > 4) {
				printf("Bad tile at %u, %u\n", x, y);
				bads++;

				for (r = 0; r < 8; r++) {
					// Paint row r
					u8 * const row = &ptr[(y * 8 + r) * pixelrow +
							x * 8 * depth];
					for (c = 0; c < 8; c++) {
						row[c * depth] = 224;
					}
				}
			}
		}
	}

	printf("\n%u bad tiles\n", bads);
}

int main(int argc, char **argv) {

	const char *name;

	Fl_File_Icon::load_system_icons();

	if (argc < 2) {
		name = fl_file_chooser("Select PNG to preview", "*.png", "", 1);
		if (!name)
			return 1;
	} else {
		name = argv[1];
	}

	Fl_PNG_Image orig(name);

	mess(&orig);

	char titlebuf[160];
	sprintf(titlebuf, "NES tile color check, %u bad tiles", bads);

	Fl_Double_Window win(orig.w(), orig.h(), titlebuf);

	Fl_Box box(0, 0, orig.w(), orig.h());

	box.image(orig);

	win.show();

	return Fl::run();
}
