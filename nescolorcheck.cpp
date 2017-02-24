#include <algorithm>
#include <lrtypes.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Box.H>
#include <stdio.h>

struct quad {
	u8 bytes[64*64*4];
	u32 i;
};

static void mess(Fl_RGB_Image *img) {

	const u8 depth = img->d();

	if (img->w() % 8 || img->h() % 8 || img->d() < 3) {
		fl_message("Error, image not divisible by 8");
		abort();
	}

	const u32 numw = img->w() / 64;
	const u32 numh = img->h() / 64;
	const u32 numq = numw * numh;

	quad *quads = new quad[numq];
	u8 * const ptr = (u8 *) img->array;

	// Read them
	const u32 pixelrow = img->w() * depth;
	u32 x, y, i;
	i = 0;
	for (y = 0; y < numh; y++) {
		for (x = 0; x < numw; x++, i++) {
			u32 r;
			for (r = 0; r < 64; r++) {
				// Copy row r
				memcpy(&quads[i].bytes[r * 64 * depth],
					&ptr[(y * 64 + r) * pixelrow +
						x * 64 * depth],
					64 * depth);
			}
			quads[i].i = i;
		}
	}

	u8 *ok = new u8[numq];
	memset(ok, 0, numq);

	// Color each unique quad red
	for (i = 1; i < numq; i++) {
		if (memcmp(quads[i - 1].bytes, quads[i].bytes, 64*64*depth) == 0) {
			ok[i - 1] = 1;
			ok[i] = 1;
		}
	}

	for (i = 0; i < numq; i++) {
		if (ok[i])
			continue;

		printf("quad %u not unique\n", quads[i].i);

		y = quads[i].i / numw;
		x = quads[i].i % numw;

		u8 *modi = ptr + y * pixelrow * 64 +
					x * 64 * depth;

		u32 c;
		for (c = 0; c < 64; c++) {
			u32 p;
			for (p = 0; p < 64; p++) {
				modi[p * depth] = 192;
			}
			modi += pixelrow;
		}
	}

	delete [] ok;
	delete [] quads;
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

	Fl_Double_Window win(orig.w(), orig.h(), "NES tile color check");

	Fl_Box box(0, 0, orig.w(), orig.h());

	box.image(orig);

	win.show();

	return Fl::run();
}
