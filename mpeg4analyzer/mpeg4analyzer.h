#ifndef __MPEG4ANALYZER_H__
#define __MPEG4ANALYZER_H__

#include "data.h"
#include "bitstream.h"
#include "global.h"
#include "macroblock.h"

#ifdef _DEBUG
enum start_code_name
{
	video_object_start_code,
	video_object_layer_start_code,
	fgs_bp_start_code,
	visual_object_sequence_start_code,
	visual_object_sequence_end_code,
	user_data_start_code,
	group_of_vop_start_code,
	video_session_error_code,
	visual_object_start_code,
	vop_start_code,
	slice_start_code,
	extension_start_code,
	fgs_vop_start_code,
	fba_object_start_code,
	fba_object_plane_start_code,
	mesh_object_start_code,
	mesh_object_plane_start_code,
	still_texture_object_start_code,
	texture_spatial_layer_start_code,
	texture_snr_layer_start_code,
	texture_tile_start_code,
	texture_shape_layer_start_code,
	stuffing_start_code,
	reserved,
	system_start_code,
	no_more_start_code
};

static string start_code_name_str [] =
{
	"video_object_start_code",
	"video_object_layer_start_code",
	"fgs_bp_start_code",
	"visual_object_sequence_start_code",
	"visual_object_sequence_end_code",
	"user_data_start_code",
	"group_of_vop_start_code",
	"video_session_error_code",
	"visual_object_start_code",
	"vop_start_code",
	"slice_start_code",
	"extension_start_code",
	"fgs_vop_start_code",
	"fba_object_start_code",
	"fba_object_plane_start_code",
	"mesh_object_start_code",
	"mesh_object_plane_start_code",
	"still_texture_object_start_code",
	"texture_spatial_layer_start_code",
	"texture_snr_layer_start_code",
	"texture_tile_start_code",
	"texture_shape_layer_start_code",
	"stuffing_start_code",
	"reserved",
	"system_start_code",
	"no_more_start_code"
};
#endif

/* start code definition */
#define VIDEO_OBJECT_START_CODE                     0x00000100  // ...1F
#define VIDEO_OBJECT_LAYER_START_CODE               0x00000120  // ...2F
#define FGS_BP_START_CODE                           0x00000140  // ...5F
#define VISUAL_OBJECT_SEQUENCE_START_CODE           0x000001B0
#define VISUAL_OBJECT_SEQUENCE_END_CODE             0x000001B1
#define USER_DATA_START_CODE                        0x000001B2
#define GROUP_OF_VOP_START_CODE                     0x000001B3
#define VIDEO_SESSION_ERROR_CODE                    0x000001B4
#define VISUAL_OBJECT_START_CODE                    0x000001B5
#define VOP_START_CODE                              0x000001B6
#define SLICE_START_CODE                            0x000001B7
#define EXTENSION_START_CODE                        0x000001B8
#define FGS_VOP_START_CODE                          0x000001B9
#define FBA_OBJECT_START_CODE                       0x000001BA
#define FBA_OBJECT_PLANE_START_CODE                 0x000001BB 
#define MESH_OBJECT_START_CODE                      0x000001BC
#define MESH_OBJECT_PLANE_START_CODE                0x000001BD
#define STILL_TEXTURE_OBJECT_START_CODE             0x000001BE
#define TEXTURE_SPATIAL_LAYER_START_CODE            0x000001BF
#define TEXTURE_SNR_LAYER_START_CODE                0x000001C0
#define TEXTURE_TILE_START_CODE                     0x000001C1
#define TEXTURE_SHAPE_LAYER_START_CODE              0x000001C2
#define STUFFING_START_CODE                         0x000001C3
//#define RESERVED,
#define SYSTEM_START_CODE                           0x000001C6 // ...FF

/* visual object type definition */
#define VISUAL_OBJECT_TYPE_VIDEO_ID				1
/*#define VISUAL_OBJECT_TYPE_STILL_TEXTURE_ID      2 */
/*#define VISUAL_OBJECT_TYPE_MESH              3 */
/*#define VISUAL_OBJECT_TYPE_FBA               4 */
/*#define VISUAL_OBJECT_TYPE_3DMESH            5 */


/*#define VIDOBJLAY_AR_SQUARE           1 */
/*#define VIDOBJLAY_AR_625TYPE_43       2 */
/*#define VIDOBJLAY_AR_525TYPE_43       3 */
/*#define VIDOBJLAY_AR_625TYPE_169      8 */
/*#define VIDOBJLAY_AR_525TYPE_169      9 */
#define VIDOBJLAY_AR_EXTPAR 0xF

#define VIDOBJLAY_SHAPE_RECTANGULAR		0
#define VIDOBJLAY_SHAPE_BINARY			1
#define VIDOBJLAY_SHAPE_BINARY_ONLY		2
#define VIDOBJLAY_SHAPE_GRAYSCALE		3

#define SPRITE_NONE		0
#define SPRITE_STATIC	1
#define SPRITE_GMC		2

#define STR(x) start_code_name_str[x]
#define READ_MARKER() BitstreamSkip(1);

/* --- macroblock modes --- */

#define MODE_INTER		0
#define MODE_INTER_Q	1
#define MODE_INTER4V	2
#define	MODE_INTRA		3
#define MODE_INTRA_Q	4
#define MODE_NOT_CODED	16
#define MODE_NOT_CODED_GMC	17

/* --- bframe specific --- */

#define MODE_DIRECT			0
#define MODE_INTERPOLATE	1
#define MODE_BACKWARD		2
#define MODE_FORWARD		3
#define MODE_DIRECT_NONE_MV	4
#define MODE_DIRECT_NO4V	5

#define I_VOP 0
#define P_VOP 1
#define B_VOP 2
#define S_VOP 3
#define N_VOP 4

static const uint8_t log2_tab_16[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

static const uint32_t intra_dc_threshold_table[] = {
	32,							/* never use */
	13,
	15,
	17,
	19,
	21,
	23,
	1,
};

/* complexity estimation toggles */
struct ESTIMATION
{
	int method;

	int opaque;
	int transparent;
	int intra_cae;
	int inter_cae;
	int no_update;
	int upsampling;

	int intra_blocks;
	int inter_blocks;
	int inter4v_blocks;
	int gmc_blocks;
	int not_coded_blocks;

	int dct_coefs;
	int dct_lines;
	int vlc_symbols;
	int vlc_bits;

	int apm;
	int npm;
	int interpolate_mc_q;
	int forw_back_mc_q;
	int halfpel2;
	int halfpel4;

	int sadct;
	int quarterpel;
};

/* resync-specific */
#define NUMBITS_VP_RESYNC_MARKER  17
#define RESYNC_MARKER 1

#define LEVELOFFSET 32

/* it's a really HUGE class, and i don't how to do with it...*/
class mpeg4analyzer : private bitstream
{
private:
	int mb_binary_shape_coding();

	int macroblock_b_vop();
	int macroblock_i_vop();
	int macroblock_p_vop(int gmc_warp_enable);

//	int macroblock(int coding_type);
	int block_intra(int i,Macroblock mb,int acpred_flag,int cbp);
	int block_inter(int cbp,int ref,int bvop);
	int block_inter_field(int cbp,int ref,int bvop);


	int video_packet_header(int coding_type, int addbits );
//	int combined_motion_shape_texture(int coding_type);
//	int motion_shape_texture(int coding_type);
	
	int GroupOfVideoObjectPlane();
	int VideoObjectPlane();
	
	int read_vop_complexity_estimation_header(int coding_type);
	int define_vop_complexity_estimation_header();
	int VideoObjectLayer();
	int VisualObject();
	int user_data();
	void VisualObjectSequence();
	uint32_t NextStartCode();

	int sprite_trajectory();
	//int brightness_change_factor();
	//int decode_sprite_piece();
	int check_resync_marker(int addbits);

	int get_mcbpc_intra();
	int get_mcbpc_inter();
	int get_cbpy(int intra);
	int get_coeff(int &run,int &last,int intra,int short_video_header);
	int get_intra_block(int direction,int start_coeff);
	int get_inter_block(int direction);

	/* Initialized once during constructor call
	* RO access is thread safe */
	REVERSE_EVENT DCT3D[2][4096];
	VLC coeff_VLC[2][2][64][64];
	void init_vlc_tables(); //it's too big to initiated by hand...


	struct Decoder
	{
		/* vol bitstream */

		int time_inc_resolution;
		int fixed_time_inc;
		uint32_t time_inc_bits;

		uint32_t shape;
		int ver_id;
		uint32_t quant_bits;
		uint32_t quant_type;
		uint16_t *mpeg_quant_matrices;
		int32_t quarterpel;
		int32_t cartoon_mode;
		int complexity_estimation_disable;
		ESTIMATION estimation;

		int interlacing;
		uint32_t top_field_first;
		uint32_t alternate_vertical_scan;

		int aspect_ratio;
		int par_width;
		int par_height;

		int sprite_enable;
		int sprite_warping_points;
		int sprite_warping_accuracy;
		int sprite_brightness_change;

		int newpred_enable;
		int reduced_resolution_enable;
		int enhancement_type;
		/* The bitstream version if it's a Xvid stream */
		int bs_version;

		/* image */

		int fixed_dimensions;
		uint32_t width;
		uint32_t height;
		uint32_t edged_width;
		uint32_t edged_height;

		//	IMAGE cur;
		//	IMAGE refn[2];				/* 0   -- last I or P VOP */
		/* 1   -- first I or P */
		//	IMAGE tmp;		/* bframe interpolation, and post processing tmp buffer */
		//	IMAGE qtmp;		/* quarter pel tmp buffer */

		/* postprocessing */
		//	XVID_POSTPROC postproc;

		/* macroblock */

		uint32_t mb_width;
		uint32_t mb_height;
		//	MACROBLOCK *mbs;

		/*
		* for B-frame & low_delay==0
		* XXX: should move frame based stuff into a DECODER_FRAMEINFO struct
		*/
		//	MACROBLOCK *last_mbs;			/* last MB */
		int last_coding_type;           /* last coding type value */
		int last_reduced_resolution;	/* last reduced_resolution value */
		int32_t frames;				/* total frame number */
		int32_t packed_mode;		/* bframes packed bitstream? (1 = yes) */
		int8_t scalability;
		//	VECTOR p_fmv, p_bmv;		/* pred forward & backward motion vector */
		int64_t time;				/* for record time */
		int64_t time_base;
		int64_t last_time_base;
		int64_t last_non_b_time;
		int32_t time_pp;
		int32_t time_bp;
		uint32_t low_delay;			/* low_delay flage (1 means no B_VOP) */
		uint32_t low_delay_default;	/* default value for low_delay flag */

		/* for GMC: central place for all parameters */

		//	IMAGE gmc;		/* gmc tmp buffer, remove for blockbased compensation */
		//	GMC_DATA gmc_data;
//	NEW_GMC_DATA new_gmc_data;

		//	xvid_image_t* out_frm;                /* This is used for slice rendering */

		int * qscale;				/* quantization table for decoder's stats */

		/* Tells if the reference image is edged or not */
		int is_edged[2];

		int num_threads;
	}Dec;
	void decode_init();

	uint32_t rounding;
	uint32_t quant;
	uint32_t fcode_forward;
	uint32_t fcode_backward;
	uint32_t intra_dc_threshold;
	WARPPOINTS gmc_warp;

	int get_dc_size_lum();
	int get_dc_size_chrom();
	int get_dc_dif(uint32_t dc_size);


	int get_mv();
	int get_motion_vector();
	int get_motion_vector_interlaced(Macroblock mb);
	int mbgmc(int cbp);
	int mb_decode( int cbp );

	int inline get_resync_len_b(const int fcode_backword,const int fcode_forward)
	{
		int resync_len = ((fcode_forward>fcode_backward) ? fcode_forward : fcode_backward) - 1;
		if (resync_len < 1) resync_len = 1;
		return resync_len;
	}
	static inline uint32_t get_dc_scaler(uint32_t quant,uint32_t lum)
	{
		if (quant < 5)
			return 8;

		if (quant < 25 && !lum)
			return (quant + 13) / 2;

		if (quant < 9)
			return 2 * quant;

		if (quant < 25)
			return quant + 8;

		if (lum)
			return 2 * quant - 16;
		else
			return quant - 6;
	}
	static uint32_t inline log2bin(uint32_t value)
	{
		int n = 0;
		if (value & 0xffff0000) {
			value >>= 16;
			n += 16;
		}
		if (value & 0xff00) {
			value >>= 8;
			n += 8;
		}
		if (value & 0xf0) {
			value >>= 4;
			n += 4;
		}
		return n + log2_tab_16[value];
	}


	ostringstream DebugLog;
	void LogToFile();


	//ostringstream blockdata;
	vector< pair<int , bits > > blockdata;
public:
	mpeg4analyzer();
	mpeg4analyzer(string strFileName);
	mpeg4analyzer(BYTE* _pData,int _length);

	~mpeg4analyzer();

	void Initialize(string strFileName);
	void Initialize(BYTE* _pData,int _length);
	
	vector< pair<int ,bits > > Analyse();

};

#endif
