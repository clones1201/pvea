#include "mpeg4analyzer.h"

mpeg4analyzer::mpeg4analyzer()
{
	bitstream();
	mpeg4analyzer::init_vlc_tables();
}

mpeg4analyzer::mpeg4analyzer(BYTE* _pData, int _length)
{
	bitstream::Initialize(_pData,_length);
	mpeg4analyzer::init_vlc_tables();
}

mpeg4analyzer::mpeg4analyzer(string _strFileName)
{
	bitstream::Initialize(_strFileName);
	mpeg4analyzer::init_vlc_tables();
}

mpeg4analyzer::~mpeg4analyzer()
{
}

void mpeg4analyzer::Initialize(BYTE* _pData, int _length)
{
	bitstream::Initialize(_pData,_length);
}

void mpeg4analyzer::Initialize(string _strFileName)
{
	bitstream::Initialize(_strFileName);
}

#ifdef _DEBUG
void mpeg4analyzer::LogToFile()
{
//	fstream file("debug_log.txt",ios_base::out);
//	file<<//DebugLog.str();
//	file.flush();
//	file.close();
	//DebugLog.clear();
}
#else if 1
void mpeg4analyzer::LogToFile()
{
	//DebugLog.clear();
}
#endif
//static 

void mpeg4analyzer::decode_init()
{
	/* For B-frame support (used to save reference frame's time */
	Dec.frames = 0;
	Dec.time = Dec.time_base = Dec.last_time_base = 0;
	Dec.low_delay = 0;
	Dec.packed_mode = 0;
	Dec.time_inc_resolution = 1; /* until VOL header says otherwise */
	Dec.ver_id = 1;

	Dec.fixed_dimensions = (Dec.width > 0 && Dec.height > 0);
}

uint32_t mpeg4analyzer::NextStartCode()
{
	enum
	{
		NOT_YET,COULD_BE_A_START_CODE,MORE_LIKELY,YES_IT_IS
	}Status;
	Status = NOT_YET;

	unsigned int buff = 0;

	try
	{
		BitstreamByteAlign();
		do
		{
			if( !is_End() )
				buff = BitstreamGetBits( 8 );
			else
				return 0; //no more start code

			switch( buff )
			{
			case 0x00:
				switch( Status )
				{
				case NOT_YET:
					Status = COULD_BE_A_START_CODE;
					break;
				case COULD_BE_A_START_CODE:
					Status = MORE_LIKELY;
					break;
				case MORE_LIKELY:
					Status = MORE_LIKELY;
					break;
				default:
					Status = NOT_YET;
					break;
				}
				break;

			case 0x01:
				switch( Status )
				{
				case NOT_YET:
					Status = NOT_YET;
					break;
				case COULD_BE_A_START_CODE:
					Status = NOT_YET;
					break;
				case MORE_LIKELY:
					Status = YES_IT_IS;
					break;
				default:
					Status = NOT_YET;
					break;
				}
				break;
			default:
				Status = NOT_YET;
				break;
			}

		}while( Status != YES_IT_IS );

		if( !is_End() )
			buff = BitstreamGetBits( 8 );
		else
			return 0;//no_more_start_code;

		//verify the start code...   may be there is a way not that ugly like this
		if( buff >= 0x00 && buff <= 0x1F)
		{
			return VIDEO_OBJECT_START_CODE;
		}
		else if(buff >= 0x20 && buff <= 0x2F)
		{
			return VIDEO_OBJECT_LAYER_START_CODE;
		}
		else if( buff >= 0x40 && buff <= 0x5F )
		{
			return FGS_BP_START_CODE;
		}
		else if( buff >= 0xC6 && buff <= 0xFF )
		{
			return SYSTEM_START_CODE;
		}
		else
		{
			return buff+=0x00000100;
		}
	}
	catch(Uninitialized)
	{
		cout<<"bitstream uninitialized..."<<endl;
	}
	return 0; // reserved
}

vector< pair<int ,bits > > mpeg4analyzer::Analyse()
{
	/*	string startCodeFileName = "start_code";

	ofstream startCodeFile(startCodeFileName,ios_base::out);
	*/
	uint32_t start_code;
	while( !mpeg4analyzer::is_End() )
	{
		BitstreamByteAlign();

		while( BitstreamShowBits(24) != 0x000001 )
			BitstreamSkip(8);
		start_code = BitstreamGetBits(32);
		//	startCodeFile<<hex<<tellg()-4<<"\t"<<STR(code_name)<<endl;

		if( start_code == VISUAL_OBJECT_SEQUENCE_START_CODE)
		{
			VisualObjectSequence();
		}

	}
	//	startCodeFile.close();

	LogToFile(); 
	return blockdata;
}

void mpeg4analyzer::VisualObjectSequence()
{
	//DebugLog<<"<visual_object_sequence>"<<endl;

	unsigned int profile_and_level_indication;
	unsigned int start_code;
	//	start_code_name code_name;

	bool isLoop = true;
	do
	{
		if( !is_End() )
			profile_and_level_indication = BitstreamGetBits(8); 
		else
			return;

		//DebugLog<<"profile_and_level_indication "<<profile_and_level_indication<<endl;
		if(profile_and_level_indication >= 0xE1 &&
			profile_and_level_indication <= 0xE8 )
		{
			//next_start_code_studio();
			//extension_and_user_data(0);
			//StudioVisualObject();
		}
		else
		{
			while(1)
			{
				if( BitstreamShowBits(32) != USER_DATA_START_CODE )
					break;
				else
					user_data();
			}

			if( BitstreamShowBits(32) != VISUAL_OBJECT_START_CODE )
				return ;

			BitstreamSkip(32);  // visual_object_start_code

			if( VisualObject() != -1 )
				;
			else
				return ;
		}

		if( BitstreamShowBits(32) != VISUAL_OBJECT_SEQUENCE_END_CODE)
		{
			isLoop = false;
		}
		else 
		{
			if( start_code == VISUAL_OBJECT_SEQUENCE_START_CODE )
			{
				BitstreamSkip(32);
				isLoop = false;
			}
			else
				return ;
		}
	}
	while( isLoop );
	BitstreamSkip(32); //visual_object_sequence_end_code
	LogToFile(); 
	return ;
}

int mpeg4analyzer::VisualObject()
{
	//DebugLog<<"<visual_object>"<<endl;

	uint32_t start_code;

	if( BitstreamGetBits(1) ) //is_visual_object_identifier
	{
		Dec.ver_id = BitstreamGetBits(4); // visual_object_verid
		//DebugLog<<"visual_object_verid "<<Dec.ver_id<<endl;

		BitstreamSkip(3);   //visual_object_priority    弃掉了的数据到底是为什么被弃掉？
	}
	else
		Dec.ver_id = 1;

	if( BitstreamShowBits(4) != VISUAL_OBJECT_TYPE_VIDEO_ID ) // visual_object_type
	{
		//DebugLog<<"ERROR: visual_object_type != video"<<endl;
		throw new Unsupported;  //only support video type
	}
	BitstreamSkip(4);

	/* video_signal_type */

	if( BitstreamGetBits(1) )   // video_signal_type
	{
		BitstreamSkip(3);  //video_format
		BitstreamSkip(1);  //video_range
		if( BitstreamGetBits(1) ) //color_description
		{
			BitstreamSkip(8);//color_primaries
			BitstreamSkip(8);//transfer_characteristics
			BitstreamSkip(8);//matrix_coefficients
		}
	}

	BitstreamByteAlign();
	while( BitstreamShowBits(24) != 0x000001 )
		BitstreamSkip(8);

	start_code = BitstreamGetBits(32);

	if( start_code == USER_DATA_START_CODE )
	{
		do
		{
			user_data();

			if( BitstreamShowBits(32) != USER_DATA_START_CODE )
			{
				BitstreamSkip(32);
			}else
				break;
		}while( 1 );
	}
	else if( start_code == VIDEO_OBJECT_START_CODE )  // video_object_start_code
	{
		if( BitstreamShowBits(32) == VIDEO_OBJECT_LAYER_START_CODE )
		{
			BitstreamSkip(32); //video_object_layer_start_code
			VideoObjectLayer();
		}
		else
		{	
			throw new Unsupported;
			//VideoObjectLayerWithShortHeader(); //shall we support the short header format??  xvid doesn't have short header
		}
	}
	else
	{
		throw new Unsupported; // not supported
	}
	BitstreamByteAlign();
	LogToFile(); 
	return 1;
}

int mpeg4analyzer::user_data()
{
	//DebugLog<<"<user_data>"<<endl;
	char data;
	while( BitstreamShowBits(24) != 0x000001 )
	{
		data = BitstreamGetBits(8);
		//DebugLog<<data;
	}
	//DebugLog<<endl;
	BitstreamByteAlign();
	LogToFile(); 
	return 0;
}

int mpeg4analyzer::VideoObjectLayer()
{

	uint32_t vol_ver_id;
	uint32_t indication;
	uint32_t low_latency_sprite_enable;
	uint32_t start_code;
	//DebugLog<<"<video_object_layer>"<<endl;

	BitstreamSkip(1); //random_accessible_vol
	indication = BitstreamGetBits(8); // video_object_type_indication
	//DebugLog<<"video_object_type_indication "<<indication<<endl;
	if(indication == 0x12 ) //  FGS is not supported
		return -1;

	if( BitstreamGetBits(1) ) //is_object_layer_identifier
	{
		//DebugLog<<" + is_object_layer_identifier"<<endl;
		vol_ver_id = BitstreamGetBits(4);
		//DebugLog<<" ver_id "<<vol_ver_id<<endl;
		//DebugLog<<"visual_object_layer_priority "<<BitstreamGetBits(3)<<endl;    //visual_object_layer_priority
	}
	else
		vol_ver_id = Dec.ver_id;

	Dec.aspect_ratio = BitstreamGetBits(4);  //aspect_ratio_info

	if( Dec.aspect_ratio == VIDOBJLAY_AR_EXTPAR )
	{
		//DebugLog<<" + aspect_ratio_info"<<endl;
		Dec.par_width = BitstreamGetBits(8); // par_width
		Dec.par_height = BitstreamGetBits(8); // par_height
	}

	if( BitstreamGetBits(1) ) // vol_control_parameters
	{
		//DebugLog<<" + vol_control_parameters"<<endl;
		BitstreamSkip(2);  // chroma_format
		Dec.low_delay = BitstreamGetBits(1); // low_delay
		//DebugLog<<"low_delay "<<Dec.low_delay<<endl;
		if( BitstreamGetBits(1) ) //vbv_parameters
		{
			unsigned int bitrate;
			unsigned int buffer_size;
			unsigned int occupancy;

			bitrate = BitstreamGetBits(15) << 15;   // first_half_bit_rate
			READ_MARKER();
			bitrate |= BitstreamGetBits(15);  // latter_half_bit_rate
			READ_MARKER();

			buffer_size = BitstreamGetBits(15) << 15; //firstr_half_vbv_buffer_size
			READ_MARKER();
			buffer_size |= BitstreamGetBits(3);  //latter_half_vbv_buffer_size

			occupancy = BitstreamGetBits(11) << 15;  //first_half_vbv_occupancy
			READ_MARKER();
			occupancy |= BitstreamGetBits(15);  //latter_half_vbv_occupancy
			READ_MARKER();

			//DebugLog<<"bitrate "<<bitrate<<"(unit=400 bps)"<<endl;
			//DebugLog<<"buffer_size"<<buffer_size<<"(unit=16384 bits)"<<endl;
			//DebugLog<<"occupancy "<<occupancy<<"(unit=64 bits)"<<endl;
		}
		else
		{
			Dec.low_delay = Dec.low_delay_default;
		}
	}
	Dec.shape = BitstreamGetBits(2);  // video_object_layer_shape
	//DebugLog<<"shape "<<Dec.shape<<endl;
	if( Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR )
	{
		//DebugLog<<"non_rectangular shapes are not supported"<<endl;
		throw new Unsupported;   //non-rectangular shapes are not supported
	}

	if( Dec.shape == VIDOBJLAY_SHAPE_RECTANGULAR && vol_ver_id != 1 )
	{
		BitstreamSkip(4);  // video_object_layer_shape_extension
	}

	READ_MARKER();

	Dec.time_inc_resolution = BitstreamGetBits(16); // vop_time_increment_resolution 
	//DebugLog<<"vop_time_increment_resolution "<<Dec.time_inc_resolution<<endl;
	if( Dec.time_inc_resolution > 0 )
	{
		Dec.time_inc_bits = MAX( log2bin(Dec.time_inc_resolution - 1),1);
	}
	else
	{
		Dec.time_inc_bits = 1;
	}

	READ_MARKER();

	if( BitstreamGetBits(1) )   //fixed_vop_rate
	{
		//DebugLog<<" + fixed_vop_rate"<<endl;
		BitstreamSkip(Dec.time_inc_bits);  // fixed_vop_time_increment
	}

	if( Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY )
	{
		if(Dec.shape == VIDOBJLAY_SHAPE_RECTANGULAR )
		{
			uint32_t width,height;

			READ_MARKER();
			width = BitstreamGetBits(13);  // video_object_layer_width  
			READ_MARKER();
			height = BitstreamGetBits(13); // video_object_layer_height
			READ_MARKER();

			//DebugLog<<"width "<<width<<endl;
			//DebugLog<<"height "<<height<<endl;

			if( Dec.width != width || Dec.height != height )
			{
				if( Dec.fixed_dimensions )  //???? I have to figure out how Dec initialized...
				{
					//DebugLog<<"analyzer width/height does not match bitstream"<<endl;
					//return -1;
				}
				Dec.width = width;
				Dec.height = height;
				Dec.fixed_dimensions = Dec.width > 0 && Dec.height > 0;
				Dec.mb_height = (Dec.height + 15) / 16;
				Dec.mb_width =  (Dec.width + 15 ) / 16;
			}
		}
		Dec.interlacing = BitstreamGetBits(1);  // interlaced
		//DebugLog<<"interlacing "<<Dec.interlacing<<endl;
		if( !BitstreamGetBits(1) )  // obmc_disable  
		{
			//DebugLog<<"obmc_disabled==false, not supported"<<endl;
			return -1; // not supported by Xvid  ,  but DivX 4.02 has this enabled 
		}

		Dec.sprite_enable = BitstreamGetBits( vol_ver_id == 1 ? 1 : 2 ); // sprite_enable

		if( Dec.sprite_enable == SPRITE_STATIC || Dec.sprite_enable == SPRITE_GMC )
		{
			int low_latenci_sprite_enable;
			if( Dec.sprite_enable != SPRITE_GMC )
			{
				BitstreamGetBits(13);   // sprite_width 
				READ_MARKER();
				BitstreamGetBits(13);   // Sprite_hetght 
				READ_MARKER();
				BitstreamGetBits(13);  // sprite_left_coordinate
				READ_MARKER();
				BitstreamGetBits(13);  // sprite_top_coordinate
				READ_MARKER();
			}

			Dec.sprite_warping_points = BitstreamGetBits(6);  //no_of_sprite_warping_points
			Dec.sprite_warping_accuracy = BitstreamGetBits(2); //sprite_warping_accuracy
			Dec.sprite_brightness_change = BitstreamGetBits(1); // brightness_change
			if (Dec.sprite_enable != SPRITE_GMC)
			{
				low_latency_sprite_enable = BitstreamGetBits(1);  // low_latency_sprite_enable 
			}
		}

		if( vol_ver_id != 1 && Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR )
		{
			BitstreamSkip(1);  // sadct_disable 
		}

		if( BitstreamGetBits(1) ) // not_8_bits
		{
			//DebugLog<<"not_8_bit==true (ignored)"<<endl;
			Dec.quant_bits = BitstreamGetBits(4);  // quant_precision
			BitstreamSkip(4); // bits_per_pixel
		}
		else
		{
			Dec.quant_bits = 5;
		}

		if( Dec.shape == VIDOBJLAY_SHAPE_GRAYSCALE )
		{
			BitstreamSkip(1);	// no_gray_quant_update 
			BitstreamSkip(1);	// composition_method 
			BitstreamSkip(1);	// linear_composition 
		}

		Dec.quant_type = BitstreamGetBits(1); // quant_type
		//DebugLog<<"quant_type "<<Dec.quant_type<<endl;

		if(Dec.quant_type)
		{
			if(BitstreamGetBits(1))  // load_intra_quant_mat
			{
				uint8_t matrix[64];
				//DebugLog<<"load_intra_quant_mat"<<endl;
				for( int i = 0 ; i < 8 ; i++ )
					BitstreamSkip(8);  // i don't decode the stream to RAW,  do i need restore the matrix??
			}


			if(BitstreamGetBits(1)) // load_inter_quant_mat 
			{
				//DebugLog<<"load_inter_quant_mat"<<endl;
				for( int i = 0 ; i < 8 ; i++ )
					BitstreamSkip(8);
			}

			if( Dec.shape == VIDOBJLAY_SHAPE_GRAYSCALE ) 
			{
				//DebugLog<<"greyscale matrix not supported"<<endl;
				throw new Unsupported; //Xvid doesn't support the greyscale matrix
			}

		}

		if( vol_ver_id != 1 )
		{
			Dec.quarterpel = BitstreamGetBits(1); //quarter_sample
			//DebugLog<<"quarterper "<<Dec.quarterpel<<endl;
		}

		Dec.complexity_estimation_disable == BitstreamGetBits(1);  // complexity_estimation_disable

		if(! Dec.complexity_estimation_disable ) 
		{
			define_vop_complexity_estimation_header();
		}

		BitstreamSkip(1);//resync_marker_disable //has to be '0'...

		if( BitstreamGetBits(1)) // data_partitioned
		{
			//DebugLog<<"Xvid does not support data_partitioned"<<endl;
			BitstreamSkip(1); //reversible_vlc
		}

		if( vol_ver_id != 1)
		{
			Dec.newpred_enable = BitstreamGetBits(1);

			if( Dec.newpred_enable ) //newpred_enable
			{
				//DebugLog<<" + newpred_enable"<<endl;
				BitstreamSkip(2);  //requested_epstream_message_type
				BitstreamSkip(1); //newpred_segment_type
			}
			Dec.reduced_resolution_enable = BitstreamGetBits(1); // reduced_resolution_vop_enable
			//DebugLog<<"reduced_resolution_enable "<<Dec.reduced_resolution_enable<<endl;
		}
		else
		{
			Dec.newpred_enable = 0; 
			Dec.reduced_resolution_enable = 0; 
		}

		Dec.scalability = BitstreamGetBits(1); // scalability
		if( Dec.scalability )
		{
			//DebugLog<<"Xvid does not support"<<endl;
			BitstreamSkip(1);	/* hierarchy_type */
			BitstreamSkip(4);	/* ref_layer_id */
			BitstreamSkip(1);	/* ref_layer_sampling_direc */
			BitstreamSkip(5);	/* hor_sampling_factor_n */
			BitstreamSkip(5);	/* hor_sampling_factor_m */
			BitstreamSkip(5);	/* vert_sampling_factor_n */
			BitstreamSkip(5);	/* vert_sampling_factor_m */
			Dec.enhancement_type = BitstreamGetBits(1);	/* enhancement_type */
			if(Dec.shape == VIDOBJLAY_SHAPE_BINARY /* && hierarchy_type==0 */) 
			{
				BitstreamSkip(1);	/* use_ref_shape */
				BitstreamSkip(1);	/* use_ref_texture */
				BitstreamSkip(5);	/* shape_hor_sampling_factor_n */
				BitstreamSkip(5);	/* shape_hor_sampling_factor_m */
				BitstreamSkip(5);	/* shape_vert_sampling_factor_n */
				BitstreamSkip(5);	/* shape_vert_sampling_factor_m */
			}
		}
	}
	else   // Dec.shape == BINARY_ONLY
	{
		if( vol_ver_id != 1)
		{
			Dec.scalability = BitstreamGetBits(1);

			if( Dec.scalability )
			{
				//DebugLog<<"Xvid does not support scalability not supported"<<endl;
				BitstreamSkip(4);	/* ref_layer_id */
				BitstreamSkip(5);	/* hor_sampling_factor_n */
				BitstreamSkip(5);	/* hor_sampling_factor_m */
				BitstreamSkip(5);	/* vert_sampling_factor_n */
				BitstreamSkip(5);	/* vert_sampling_factor_m */
			}
		}
		BitstreamSkip(1);  // resync_marker_disable  
	}

	BitstreamByteAlign();

	start_code = BitstreamShowBits(32);
	if( start_code == USER_DATA_START_CODE )
	{
		BitstreamSkip(32);
		do
		{
			user_data();

			if( BitstreamShowBits(32) == USER_DATA_START_CODE )
			{
				BitstreamSkip(32);
			}else
				break;
		}while( 1 );
	}

	if( Dec.sprite_enable == SPRITE_STATIC && !low_latency_sprite_enable )
	{
		BitstreamSkip(32);  // vop_start_code
		VideoObjectPlane();
	}

	int coding_type;
	do
	{
		if( BitstreamShowBits(32) == GROUP_OF_VOP_START_CODE )
		{
			BitstreamSkip(32);  //group_of_vop_start_code
			GroupOfVideoObjectPlane();
		}
		
		BitstreamSkip(32);  //vop_start_code
		coding_type = VideoObjectPlane();

		if(( coding_type == B_VOP || coding_type == S_VOP || Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR )
			&& BitstreamShowBits(32) == STUFFING_START_CODE )
		{
			BitstreamSkip(32);
			while( BitstreamShowBits(24) != 0x000001 )
				BitstreamSkip(8);
		}

		BitstreamByteAlign();

		do
		{

			while( BitstreamShowBits(24) != 0x000001 && !is_End() )
				BitstreamSkip(8);
			
			if( is_End() )
				return 0;

			if(BitstreamShowBits(32) != GROUP_OF_VOP_START_CODE && 
				BitstreamShowBits(32) != VOP_START_CODE)
				BitstreamSkip(24);
			else if( BitstreamShowBits(32) == VISUAL_OBJECT_SEQUENCE_START_CODE )
				break;
			else
				break;

		}while(1);
	}while( BitstreamShowBits(32) == GROUP_OF_VOP_START_CODE || 
		BitstreamShowBits(32) == VOP_START_CODE );

	LogToFile(); 
	return 0;
}

int mpeg4analyzer::GroupOfVideoObjectPlane()
{
	//DebugLog<<"<group_of_vop>"<<endl;


	uint32_t start_code;
	int hours,minutes,seconds;
	/* time code */
	hours = BitstreamGetBits(5); //hours
	minutes = BitstreamGetBits(6); //minutes
	READ_MARKER();
	seconds = BitstreamGetBits(6); //seconds
	//DebugLog<<"time "<<hours<<"h"<<minutes<<"m"<<seconds<<"s"<<endl;

	BitstreamSkip(1); // closed_gov
	BitstreamSkip(1); // broken_link

	BitstreamByteAlign();
	start_code = BitstreamShowBits(32);
	if( start_code == USER_DATA_START_CODE )
	{
		BitstreamSkip(32);
		do
		{
			user_data();

			if( BitstreamShowBits(32) != USER_DATA_START_CODE )
			{
				BitstreamSkip(32);
			}else
				break;
		}while( 1 );
	}
	LogToFile(); 
	return 0;
}

int mpeg4analyzer::VideoObjectPlane()
{
	

	//DebugLog<<"<vop>"<<" addr: 0x"<<hex<<tellg()<<dec<<endl;
	int coding_type = BitstreamGetBits(2); // vop_coding_type

	//DebugLog<<"coding_type "<<coding_type<<endl;
	int modulo_time_type;
	int time_incr = 0;
	int time_increment = 0;

	while( BitstreamGetBits(1) != 0 ) // modulo_time_base
		time_incr++;

	READ_MARKER();

	if( Dec.time_inc_bits )
	{
		time_increment = BitstreamGetBits( Dec.time_inc_bits );  // vop_time_increment
	}

	//DebugLog<<"time_base "<<time_incr<<endl<<"time_increment "<<time_increment<<endl;

	if (coding_type != B_VOP) 
	{
		Dec.last_time_base = Dec.time_base;
		Dec.time_base += time_incr;
		Dec.time = Dec.time_base*Dec.time_inc_resolution + time_increment;
		Dec.time_pp = (int32_t)(Dec.time - Dec.last_non_b_time);
		Dec.last_non_b_time = Dec.time;
	} 
	else 
	{
		Dec.time = (Dec.last_time_base + time_incr)*Dec.time_inc_resolution + time_increment;
		Dec.time_bp = Dec.time_pp - (int32_t)(Dec.last_non_b_time - Dec.time);
	}
	if (Dec.time_pp <= 0) Dec.time_pp = 1;
	//DebugLog<<"time_pp="<<Dec.time_pp<<endl;
	//DebugLog<<"time_bp="<<Dec.time_bp<<endl;

	READ_MARKER();

	if( !BitstreamGetBits(1) )   //vop_coded
	{
		//DebugLog<<"vop_coded=false"<<endl;
		return N_VOP;
	}

	if( Dec.newpred_enable )
	{
		int vop_id;
		int vop_id_for_prediction;
		vop_id = BitstreamGetBits(MIN(Dec.time_inc_bits + 3, 15)); //vop_id
		//DebugLog<<"vop_id "<<vop_id<<endl;
		if( BitstreamGetBits(1) )
		{
			vop_id_for_prediction = BitstreamGetBits( MIN( Dec.time_inc_bits + 3 , 15));
			//DebugLog<<"vop_id_for_prediction "<<vop_id_for_prediction<<endl;
		}
		READ_MARKER();
	}

	//comment from xvid code /* fix a little bug by MinChen <chenm002@163.com> */  // what bug?
	if ((Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY) &&
		( (coding_type == P_VOP) || (coding_type == S_VOP && Dec.sprite_enable == SPRITE_GMC) ) ) 
	{
		rounding = BitstreamGetBits(1);	/* vop_rounding_type */
		//DebugLog<<"rounding "<<rounding<<endl;
	}

	if (Dec.reduced_resolution_enable &&
		Dec.shape == VIDOBJLAY_SHAPE_RECTANGULAR &&
		(coding_type == P_VOP || coding_type == I_VOP)) 
	{
		if (BitstreamGetBits(1)); // vop_reduced_resolution  ?? what for?
		////DebugLog<<"Xvid does not support RRV (anymore)"<<endl;
	}

	if (Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR) 
	{
		if(!(Dec.sprite_enable == SPRITE_STATIC && coding_type == I_VOP)) 
		{
			uint32_t width,height;
			uint32_t horiz_mc_ref,vert_mc_ref;

			width = BitstreamGetBits(13);  //vop_width
			READ_MARKER();
			height = BitstreamGetBits(13);  //vop_height
			READ_MARKER();
			horiz_mc_ref = BitstreamGetBits(13);  //vop_horizontal_mc_spatial_ref
			READ_MARKER();
			vert_mc_ref = BitstreamGetBits(13); // vop_vertical_mc_spatial_ref
			READ_MARKER();

			//DebugLog<<"width "<<width<<endl<<"height "<<height<<endl<<"horiz_mc_ref<<horiz_mc_ref<<endl<<"vert_mc_ref "<<vert_mc_ref<<endl;

		}

		if(( Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY ) && Dec.scalability && Dec.enhancement_type )
			BitstreamGetBits(1);  // background_composition

		BitstreamSkip(1);	/* change_conv_ratio_disable */
		if (BitstreamGetBits(1))	/* vop_constant_alpha */
		{
			BitstreamSkip(8);	/* vop_constant_alpha_value */
		}
	}

	if (Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY) 
	{

		if (!Dec.complexity_estimation_disable)
		{
			read_vop_complexity_estimation_header(coding_type);
		}

		/* intra_dc_vlc_threshold */
		intra_dc_threshold =
			intra_dc_threshold_table[BitstreamGetBits(3)];

		Dec.top_field_first = 0;
		Dec.alternate_vertical_scan = 0;

		if (Dec.interlacing) 
		{
			Dec.top_field_first = BitstreamGetBits(1);  // top_feild_first
			//DebugLog<<"interlace top_fied_first "<<Dec.top_field_first<<endl;

			Dec.alternate_vertical_scan = BitstreamGetBits(1); // alternate_vertical_sacn_flag
			//DebugLog<<"intrelace alternate_vertical_scan "<<Dec.alternate_vertical_scan<<endl;

		}
	}

	if ((Dec.sprite_enable == SPRITE_STATIC || Dec.sprite_enable== SPRITE_GMC) && coding_type == S_VOP)
	{
		for (int i = 0 ; i < Dec.sprite_warping_points; i++)
		{
			int length;
			int x = 0, y = 0;

			/* sprite code borowed from ffmpeg; thx Michael Niedermayer <michaelni@gmx.at> */
			length = sprite_trajectory();
			
			if(length)
			{
				x= BitstreamGetBits(length);
				if ((x >> (length - 1)) == 0) /* if MSB not set it is negative*/
					x = - (x ^ ((1 << length) - 1));
			}

			READ_MARKER();

			length = sprite_trajectory();
			
			if(length)
			{
				y = BitstreamGetBits( length );
				if ((y >> (length - 1)) == 0) /* if MSB not set it is negative*/
					y = - (y ^ ((1 << length) - 1));
			}
			READ_MARKER();

			gmc_warp.duv[i].x = x;
			gmc_warp.duv[i].y = y;

		}

		if (Dec.sprite_brightness_change)
		{
			/* XXX: brightness_change_factor() */
		}
		if (Dec.sprite_enable == SPRITE_STATIC)
		{
			/* XXX: todo */
		}

	}

	if(Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY )
	{
		if ((quant = BitstreamGetBits(Dec.quant_bits)) < 1)	/* vop_quant */
			quant = 1;
		//DebugLog<<"quant "<<quant<<endl;

		if (coding_type != I_VOP) 
		{
			fcode_forward = BitstreamGetBits(3);	/* fcode_forward */
			//DebugLog<<"fcode_forward "<<fcode_forward<<endl;
		}

		if (coding_type == B_VOP)
		{
			fcode_backward = BitstreamGetBits(3);	/* fcode_backward */
			//DebugLog<<"fcode_backward "<<fcode_backward<<endl;
		}
		if (!Dec.scalability) 
		{
			if ((Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR) &&
				(coding_type != I_VOP)) 
			{
				BitstreamSkip(1);	/* vop_shape_coding_type */
			}

			if( coding_type != B_VOP)
			{
				switch(coding_type)
				{
				case I_VOP:
					macroblock_i_vop();
					break;
				case P_VOP:
					macroblock_p_vop(0);
					break;
				case S_VOP:
					macroblock_p_vop(1);
					break;
				default:
					break;
				}
			}else /* B_VOP */
			{
			
			}
/*
			motion_shape_texture(coding_type);

			switch(coding_type)
			{
			case I_VOP:
				while( check_resync_marker(0) )
				{
					BitstreamByteAlign();
					BitstreamSkip( NUMBITS_VP_RESYNC_MARKER + 0 ); //resync_marker
					video_packet_header(coding_type);
					motion_shape_texture(coding_type);
				}
				break;
			case P_VOP:
				while( check_resync_marker( fcode_forward - 1))
				{
					BitstreamByteAlign();
					BitstreamSkip( NUMBITS_VP_RESYNC_MARKER + fcode_forward - 1);
					video_packet_header( coding_type );
					motion_shape_texture( coding_type );
				}
				break;
			case S_VOP:
				while( check_resync_marker( fcode_forward - 1))
				{
					BitstreamByteAlign();
					BitstreamSkip( NUMBITS_VP_RESYNC_MARKER + fcode_forward - 1);
					video_packet_header( coding_type );
					motion_shape_texture( coding_type );
				}
				break;
			case B_VOP:

				while( check_resync_marker( get_resync_len_b(fcode_forward,fcode_backward)) )
				{
					BitstreamByteAlign();
					BitstreamSkip( get_resync_len_b(fcode_forward,fcode_backward) );
					video_packet_header( coding_type );
					motion_shape_texture( coding_type );
				}
				break;
			default:
				break;
			}*/
		}
		else
		{
			/* enhancement_type is not supported by Xvid */
		//	combined_motion_shape_texture(coding_type);
		}
	}
	else
	{
		/* Xvid does not have this branch. 
		and i don't know how to continue 
		without the value of fcode */
	}
	LogToFile(); 

	cout<<int( double(tellg())/double(GetLength()) * 100 )<<"%\r";

	return coding_type;

}

/*
int mpeg4analyzer::motion_shape_texture(int coding_type)
{
	//data_partitioned is not supported
	return combined_motion_shape_texture(coding_type);
}*/

int mpeg4analyzer::video_packet_header(int current_coding_type,int addbits )
{
	int hec = 0;
	int mbnum;
	int mbnum_bits = log2bin( Dec.mb_width * Dec.mb_height - 1);  // how do we know it ???

	BitstreamByteAlign();
	BitstreamSkip( NUMBITS_VP_RESYNC_MARKER + addbits);

	//DebugLog<<"<video_packet_header>"<<endl;
	if (Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR)
	{
		hec = BitstreamGetBits(1);		/* header_extension_code */
		if (hec && !( Dec.sprite_enable == SPRITE_STATIC && current_coding_type == I_VOP ))
		{
			BitstreamSkip(13);			/* vop_width */
			READ_MARKER();
			BitstreamSkip(13);			/* vop_height */
			READ_MARKER();
			BitstreamSkip(13);			/* vop_horizontal_mc_spatial_ref */
			READ_MARKER();
			BitstreamSkip(13);			/* vop_vertical_mc_spatial_ref */
			READ_MARKER();
		}
	}

	mbnum = BitstreamGetBits(mbnum_bits);		/* macroblock_number */
	//DebugLog<<"mbnum "<<mbnum<<endl;

	if (Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
	{
		quant = BitstreamGetBits(Dec.quant_bits);	/* quant_scale */
		//DebugLog<<"quant "<<quant<<endl;
	}

	if (Dec.shape == VIDOBJLAY_SHAPE_RECTANGULAR)
		hec = BitstreamGetBits(1);		/* header_extension_code */


	//DebugLog<<"header_extension_code "<<hec<<endl;

	if (hec)
	{
		int time_base;
		int time_increment;
		int coding_type;

		for (time_base=0; BitstreamGetBits(1)!=0; time_base++);		/* modulo_time_base */
		READ_MARKER();
		if (Dec.time_inc_bits)
			time_increment = (BitstreamGetBits(Dec.time_inc_bits));	/* vop_time_increment */
		READ_MARKER();
		//DebugLog<<"time "<<time_base<<":"<<time_increment<<endl;

		coding_type = BitstreamGetBits(2);
		//DebugLog<<"coding_type "<<coding_type<<endl;

		if (Dec.shape != VIDOBJLAY_SHAPE_RECTANGULAR)
		{
			BitstreamSkip(1);	/* change_conv_ratio_disable */
			if (coding_type != I_VOP)
				BitstreamSkip(1);	/* vop_shape_coding_type */
		}

		if (Dec.shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
		{
			intra_dc_threshold = intra_dc_threshold_table[BitstreamGetBits(3)];

			if (Dec.sprite_enable == SPRITE_GMC && coding_type == S_VOP &&
				Dec.sprite_warping_points > 0)
			{
				/* TODO: sprite trajectory */
				for (int i = 0 ; i < Dec.sprite_warping_points; i++)
				{
					int length;
					int x = 0, y = 0;

					/* sprite code borowed from ffmpeg; thx Michael Niedermayer <michaelni@gmx.at> */
					length = sprite_trajectory();

					if(length)
					{
						x= BitstreamGetBits(length);
						if ((x >> (length - 1)) == 0) /* if MSB not set it is negative*/
							x = - (x ^ ((1 << length) - 1));
					}

					READ_MARKER();

					length = sprite_trajectory();

					if(length)
					{
						y = BitstreamGetBits( length );
						if ((y >> (length - 1)) == 0) /* if MSB not set it is negative*/
							y = - (y ^ ((1 << length) - 1));
					}
					READ_MARKER();

					gmc_warp.duv[i].x = x;
					gmc_warp.duv[i].y = y;

				}

			}
			if (Dec.reduced_resolution_enable &&
				Dec.shape == VIDOBJLAY_SHAPE_RECTANGULAR &&
				(coding_type == P_VOP || coding_type == I_VOP))
			{
				BitstreamSkip(1); /* XXX: vop_reduced_resolution */
			}

			if (coding_type != I_VOP && fcode_forward)
			{
				fcode_forward = BitstreamGetBits(3);
				//DebugLog<<"fcode_forward "<<fcode_forward<<endl;
			}

			if (coding_type == B_VOP && fcode_backward)
			{
				fcode_backward = BitstreamGetBits(3);
				//DebugLog<<"fcode_backward "<<fcode_backward<<endl;
			}
		}
	}

	if (Dec.newpred_enable)
	{
		int vop_id;
		int vop_id_for_prediction;

		vop_id = BitstreamGetBits(MIN(Dec.time_inc_bits + 3, 15));
		//DebugLog<<"vop_id "<<vop_id<<endl;
		if (BitstreamGetBits(1))	/* vop_id_for_prediction_indication */
		{
			vop_id_for_prediction = BitstreamGetBits(MIN(Dec.time_inc_bits + 3, 15));
			//DebugLog<<"vop_id_for_prediction "<<vop_id_for_prediction<<endl;
		}
		READ_MARKER();
	}

	return mbnum;

}

/*
 * for IVOP addbits == 0
 * for PVOP addbits == fcode - 1
 * for BVOP addbits == max(fcode,bcode) - 1
 * returns true or false
 */
int mpeg4analyzer::check_resync_marker(int addbits)
{
	uint32_t nbits;
	uint32_t code;
	uint32_t nbitsresyncmarker = NUMBITS_VP_RESYNC_MARKER + addbits;

	nbits = BitstreamNumBitsToByteAlign();
	code = BitstreamShowBits(nbits);

	if (code == (((uint32_t)1 << (nbits - 1)) - 1)) // checking stuffing code?
	{
		return BitstreamShowBitsFromByteAlign(nbitsresyncmarker) == RESYNC_MARKER;
	}

	return 0;
}

int mpeg4analyzer::sprite_trajectory()
{
	int i;
	for (i = 0; i < 12; i++)
	{
		if (BitstreamShowBits(sprite_trajectory_len[i].len) == sprite_trajectory_len[i].code)
		{
			BitstreamSkip(sprite_trajectory_len[i].len);
			return i;
		}
	}
	return -1;
}
int mpeg4analyzer::read_vop_complexity_estimation_header(int coding_type)
{
	ESTIMATION * e = &Dec.estimation;

	if (e->method == 0 || e->method == 1)
	{
		if (coding_type == I_VOP) {
			if (e->opaque)		BitstreamSkip(8);	/* dcecs_opaque */
			if (e->transparent) BitstreamSkip(8);	/* */  //they simply ignored it...
			if (e->intra_cae)	BitstreamSkip(8);	/* */
			if (e->inter_cae)	BitstreamSkip(8);	/* */
			if (e->no_update)	BitstreamSkip(8);	/* */
			if (e->upsampling)	BitstreamSkip(8);	/* */
			if (e->intra_blocks) BitstreamSkip(8);	/* */
			if (e->not_coded_blocks) BitstreamSkip(8);	/* */
			if (e->dct_coefs)	BitstreamSkip(8);	/* */
			if (e->dct_lines)	BitstreamSkip(8);	/* */
			if (e->vlc_symbols) BitstreamSkip(8);	/* */
			if (e->vlc_bits)	BitstreamSkip(8);	/* */
			if (e->sadct)		BitstreamSkip(8);	/* */
		}

		if (coding_type == P_VOP) {
			if (e->opaque) BitstreamSkip(8);		/* */
			if (e->transparent) BitstreamSkip(8);	/* */
			if (e->intra_cae)	BitstreamSkip(8);	/* */
			if (e->inter_cae)	BitstreamSkip(8);	/* */
			if (e->no_update)	BitstreamSkip(8);	/* */
			if (e->upsampling) BitstreamSkip(8);	/* */
			if (e->intra_blocks) BitstreamSkip(8);	/* */
			if (e->not_coded_blocks)	BitstreamSkip(8);	/* */
			if (e->dct_coefs)	BitstreamSkip(8);	/* */
			if (e->dct_lines)	BitstreamSkip(8);	/* */
			if (e->vlc_symbols) BitstreamSkip(8);	/* */
			if (e->vlc_bits)	BitstreamSkip(8);	/* */
			if (e->inter_blocks) BitstreamSkip(8);	/* */
			if (e->inter4v_blocks) BitstreamSkip(8);	/* */
			if (e->apm)			BitstreamSkip(8);	/* */
			if (e->npm)			BitstreamSkip(8);	/* */
			if (e->forw_back_mc_q) BitstreamSkip(8);	/* */
			if (e->halfpel2)	BitstreamSkip(8);	/* */
			if (e->halfpel4)	BitstreamSkip(8);	/* */
			if (e->sadct)		BitstreamSkip(8);	/* */
			if (e->quarterpel)	BitstreamSkip(8);	/* */
		}
		if (coding_type == B_VOP) {
			if (e->opaque)		BitstreamSkip(8);	/* */
			if (e->transparent)	BitstreamSkip(8);	/* */
			if (e->intra_cae)	BitstreamSkip(8);	/* */
			if (e->inter_cae)	BitstreamSkip(8);	/* */
			if (e->no_update)	BitstreamSkip(8);	/* */
			if (e->upsampling)	BitstreamSkip(8);	/* */
			if (e->intra_blocks) BitstreamSkip(8);	/* */
			if (e->not_coded_blocks) BitstreamSkip(8);	/* */
			if (e->dct_coefs)	BitstreamSkip(8);	/* */
			if (e->dct_lines)	BitstreamSkip(8);	/* */
			if (e->vlc_symbols)	BitstreamSkip(8);	/* */
			if (e->vlc_bits)	BitstreamSkip(8);	/* */
			if (e->inter_blocks) BitstreamSkip(8);	/* */
			if (e->inter4v_blocks) BitstreamSkip(8);	/* */
			if (e->apm)			BitstreamSkip(8);	/* */
			if (e->npm)			BitstreamSkip(8);	/* */
			if (e->forw_back_mc_q) BitstreamSkip(8);	/* */
			if (e->halfpel2)	BitstreamSkip(8);	/* */
			if (e->halfpel4)	BitstreamSkip(8);	/* */
			if (e->interpolate_mc_q) BitstreamSkip(8);	/* */
			if (e->sadct)		BitstreamSkip(8);	/* */
			if (e->quarterpel)	BitstreamSkip(8);	/* */
		}

		if (coding_type == S_VOP && Dec.sprite_enable == SPRITE_STATIC) {
			if (e->intra_blocks) BitstreamSkip(8);	/* */
			if (e->not_coded_blocks) BitstreamSkip(8);	/* */
			if (e->dct_coefs)	BitstreamSkip(8);	/* */
			if (e->dct_lines)	BitstreamSkip(8);	/* */
			if (e->vlc_symbols)	BitstreamSkip(8);	/* */
			if (e->vlc_bits)	BitstreamSkip(8);	/* */
			if (e->inter_blocks) BitstreamSkip(8);	/* */
			if (e->inter4v_blocks)	BitstreamSkip(8);	/* */
			if (e->apm)			BitstreamSkip(8);	/* */
			if (e->npm)			BitstreamSkip(8);	/* */
			if (e->forw_back_mc_q)	BitstreamSkip(8);	/* */
			if (e->halfpel2)	BitstreamSkip(8);	/* */
			if (e->halfpel4)	BitstreamSkip(8);	/* */
			if (e->interpolate_mc_q) BitstreamSkip(8);	/* */
		}
	}
	return 0;
}

int mpeg4analyzer::define_vop_complexity_estimation_header()
{
	ESTIMATION * e = &Dec.estimation;

	e->method = BitstreamGetBits(2);	/* estimation_method */

	if (e->method == 0 || e->method == 1)
	{
		if (!BitstreamGetBits(1))		/* shape_complexity_estimation_disable */
		{
			e->opaque = BitstreamGetBits(1);		/* opaque */
			e->transparent = BitstreamGetBits(1);		/* transparent */
			e->intra_cae = BitstreamGetBits(1);		/* intra_cae */
			e->inter_cae = BitstreamGetBits(1);		/* inter_cae */
			e->no_update = BitstreamGetBits(1);		/* no_update */
			e->upsampling = BitstreamGetBits(1);		/* upsampling */
		}

		if (!BitstreamGetBits(1))	/* texture_complexity_estimation_set_1_disable */
		{
			e->intra_blocks = BitstreamGetBits(1);		/* intra_blocks */
			e->inter_blocks = BitstreamGetBits(1);		/* inter_blocks */
			e->inter4v_blocks = BitstreamGetBits(1);		/* inter4v_blocks */
			e->not_coded_blocks = BitstreamGetBits(1);		/* not_coded_blocks */
		}
	}

	READ_MARKER();

	if (!BitstreamGetBits(1))		/* texture_complexity_estimation_set_2_disable */
	{
		e->dct_coefs = BitstreamGetBits(1);		/* dct_coefs */
		e->dct_lines = BitstreamGetBits(1);		/* dct_lines */
		e->vlc_symbols = BitstreamGetBits(1);		/* vlc_symbols */
		e->vlc_bits = BitstreamGetBits(1);		/* vlc_bits */
	}

	if (!BitstreamGetBits(1))		/* motion_compensation_complexity_disable */
	{
		e->apm = BitstreamGetBits(1);		/* apm */
		e->npm = BitstreamGetBits(1);		/* npm */
		e->interpolate_mc_q = BitstreamGetBits(1);		/* interpolate_mc_q */
		e->forw_back_mc_q = BitstreamGetBits(1);		/* forw_back_mc_q */
		e->halfpel2 = BitstreamGetBits(1);		/* halfpel2 */
		e->halfpel4 = BitstreamGetBits(1);		/* halfpel4 */
	}

	READ_MARKER();

	if (e->method == 1)
	{
		if (!BitstreamGetBits(1))	/* version2_complexity_estimation_disable */
		{
			e->sadct = BitstreamGetBits(1);		/* sadct */
			e->quarterpel = BitstreamGetBits(1);		/* quarterpel */
		}
	}
	return 0;
}

/*
int mpeg4analyzer::combined_motion_shape_texture(int coding_type)
{
	bool sign = false;
	do
	{
		
		macroblock(coding_type);


		{
			switch(coding_type)
			{
			case I_VOP:
				sign = ! check_resync_marker(0);
				break;

			case P_VOP:
				sign = ! check_resync_marker(fcode_forward - 1);
				break;

			case S_VOP:
				sign = ! check_resync_marker(fcode_forward - 1);
				break;

			case B_VOP:
				sign = ! check_resync_marker( get_resync_len_b(fcode_forward,fcode_backward) );
				break;

			default:
				break;
			}
			sign = sign && (BitstreamShowBitsFromByteAlign(23)!=0);
			sign = sign || valid_stuffing_bits();
		}
	}while( sign );
	return 0;
}
*/

/*
int mpeg4analyzer::macroblock(int coding_type)
{
	int not_coded = 0;
	int mcbpc;
	switch(coding_type)
	{
	case I_VOP:
		macroblock_i_vop();
		break;
	case P_VOP:
		macroblock_p_vop();
		break;
	case S_VOP:
		macroblock_p_vop();
		break;
	case B_VOP:
		macroblock_b_vop();
		break;
	case N_VOP:
		break;
	default:
		break;
	}

	return 0;
}
*/

int mpeg4analyzer::get_mcbpc_inter()
{
	uint32_t index;

	index = MIN( BitstreamShowBits(9), 256 );

	BitstreamSkip(mcbpc_inter_table[index].len);

	return mcbpc_inter_table[index].code;
}

int mpeg4analyzer::get_mcbpc_intra()
{
	uint32_t index;

	index = BitstreamShowBits(9);
	index >>= 3;

	BitstreamSkip(mcbpc_intra_table[index].len);

	return mcbpc_intra_table[index].code;
}


static const int32_t dquant_table[4] = {
  -1, -2, 1, 2
};

static int block_count = 6;
/*
NOTE :  The value of block_count is 6 in the 4:2:0 format.  
NOTE :  The value of alpha_block_count is 4. 
*/


int mpeg4analyzer::macroblock_i_vop()
{
	uint32_t bound;
	uint32_t x,y;
	const uint32_t mb_width = Dec.mb_width;
	const uint32_t mb_height = Dec.mb_height;

	bound = 0;

	for( y = 0 ; y < mb_height ; y++ )
	{
		for( x = 0 ; x < mb_width ; x++ )
		{
			Macroblock mb;
			uint32_t mcbpc;
			uint32_t cbpc;
			uint32_t acpred_flag;
			uint32_t cbpy;
			uint32_t cbp;

			while( BitstreamShowBits(9) == 1)
				BitstreamSkip(9);

			if( check_resync_marker(0) )
			{
				bound = video_packet_header(I_VOP,0);
				x = bound % mb_width;
				y = MIN( bound / mb_width , mb_height - 1 );
			}

			//DebugLog<<"macroblock ("<<x<<","<<y<<") 0x"<<hex<<BitstreamShowBits(32)<<endl;
			//DebugLog<<dec;
			mcbpc = get_mcbpc_intra();  // mcbpc , VLC
			mb.mode = mcbpc & 7;
			cbpc = (mcbpc >> 4);

			acpred_flag = BitstreamGetBits(1); // acpred_flag

			cbpy = get_cbpy(1); //cbpy , VLC
			cbp = (cbpy << 2) | cbpc;
			//DebugLog<<"cbp "<<cbp<<endl;
			if( mb.mode == MODE_INTRA_Q )
			{
				quant += dquant_table[BitstreamGetBits(2)];  //dquant
				if( quant > 31 )
					quant = 31;
				else if(quant < 1)
					quant = 1;
			}
			mb.quant = quant;
			//DebugLog<<"quantizer "<<mb.quant<<endl;

			mb.mvs[0].x = mb.mvs[0].y =
				mb.mvs[1].x = mb.mvs[1].y =
				mb.mvs[2].x = mb.mvs[2].y =
				mb.mvs[3].x = mb.mvs[3].y =0;
			if(Dec.interlacing)
			{
				mb.field_dct = BitstreamGetBits(1);
				//DebugLog<<"deci:field_dct "<<mb.field_dct<<endl;
			}

			for( int i = 0 ; i < 6 ; i++ )
			{
				block_intra(i,mb,acpred_flag,cbp);
			}
		}
	}
	LogToFile();
	return 0;
}

int mpeg4analyzer::macroblock_p_vop(int gmc_warp_enable)
{
	uint32_t x,y;
	uint32_t bound;
	Macroblock mb;
	int cp_mb,st_mb;
	const uint32_t mb_width = Dec.mb_width;
	const uint32_t mb_height = Dec.mb_height;

	for( y = 0 ; y < mb_height ; y ++ )
	{
		cp_mb = st_mb = 0;
		for( x = 0 ; x < mb_width ; x ++ )
		{
			/*  Skip stuffing */
			while( BitstreamShowBits(10) == 1 )
				BitstreamSkip(10);

			if( check_resync_marker(fcode_forward - 1))
			{
				bound = video_packet_header( P_VOP,fcode_forward - 1 );
				x = bound % mb_width;
				y = MIN( bound / mb_width , mb_height - 1);
			}

			//DebugLog<<"macroblock ("<<x<<","<<y<<") 0x"<<hex<<BitstreamShowBits(32)<<dec<<endl;

			if (!(BitstreamGetBits(1))) 
			{ /* block _is_ coded */
				uint32_t mcbpc, cbpc, cbpy, cbp;
				uint32_t intra, acpred_flag = 0;
				int mcsel = 0;    /* mcsel: '0'=local motion, '1'=GMC */

				cp_mb++;
				mcbpc = get_mcbpc_inter();
				mb.mode = mcbpc & 7;
				cbpc = (mcbpc >> 4);

				//DebugLog<<"mode "<<mb.mode<<endl;
		
				intra = (mb.mode == MODE_INTRA || mb.mode == MODE_INTRA_Q);

				if (gmc_warp_enable && (mb.mode == MODE_INTER || mb.mode == MODE_INTER_Q))
					mcsel = BitstreamGetBits(1);
				else if (intra)
					acpred_flag = BitstreamGetBits(1);

				cbpy = get_cbpy(intra);
				cbp = (cbpy << 2) | cbpc;
				//DebugLog<<"cbp "<<cbp<<endl;

				if (mb.mode == MODE_INTER_Q || mb.mode == MODE_INTRA_Q) {
					int dquant = dquant_table[BitstreamGetBits(2)];
					//DebugLog<<"dquant "<<dquant<<endl;
					quant += dquant;
					if (quant > 31) {
						quant = 31;
					} else if (quant < 1) {
						quant = 1;
					}
				
				}
				mb.quant = quant;
				//DebugLog<<"quant "<<mb.quant<<endl;
				mb.field_pred=0;
				/* interlacing_information  */
				if (Dec.interlacing)
				{
					if (cbp || intra)
					{
						mb.field_dct = BitstreamGetBits(1);
						//DebugLog<<"decp: field_dct: "<<mb.field_dct<<endl;
					}

					if ((mb.mode == MODE_INTER || mb.mode == MODE_INTER_Q) && !mcsel) {
						mb.field_pred = BitstreamGetBits(1);
						//DebugLog<<"decp: field_pred: "<<mb.field_pred<<endl;

						if (mb.field_pred) {
							mb.field_for_top = BitstreamGetBits(1);
							//DebugLog<<"decp: field_for_top: "<<mb.field_for_top<<endl;
							mb.field_for_bot = BitstreamGetBits(1);
							//DebugLog<<"decp: field_for_bot: "<<mb.field_for_bot<<endl;
						}
					}
				}

				if(mcsel)
				{
					mbgmc(cbp);
					continue;
				}
				else if (mb.mode == MODE_INTER || mb.mode == MODE_INTER_Q) 
				{
					if(Dec.interlacing) {
						/* Get motion vectors interlaced, field_pred is handled there */
						get_motion_vector_interlaced(mb);
					} else {
						get_motion_vector();
						mb.mvs[1] = mb.mvs[2] = mb.mvs[3] = mb.mvs[0];
					}
				} else if (mb.mode == MODE_INTER4V ) {
					/* interlaced missing here */
					get_motion_vector();
					get_motion_vector();
					get_motion_vector();
					get_motion_vector();
				} else { /* MODE_INTRA, MODE_INTRA_Q */
					mb.mvs[0].x = mb.mvs[1].x = mb.mvs[2].x = mb.mvs[3].x = 0;
					mb.mvs[0].y = mb.mvs[1].y = mb.mvs[2].y = mb.mvs[3].y = 0;
					for( int i = 0 ; i < 6 ; i++ )
					{
						block_intra( i , mb,acpred_flag, cbp);
					}
					continue;
				}

				/* See how to decode */
				if(!mb.field_pred)
				{ 
						block_inter( cbp, 0, 0);
				}
				else 
				{
						block_inter_field( cbp, 0, 0);
				}
			}
		    else if (gmc_warp_enable) 
			{  /* a not coded S(GMC)-VOP macroblock */
				mb.mode = MODE_NOT_CODED_GMC;
				mb.quant = quant;
				//DebugLog<<"mode "<<mb.mode<<endl;
				//DebugLog<<"quant "<<mb.quant<<endl;

				mbgmc(0x00);

				st_mb = x + 1;
		    } 
			else 
			{ /* not coded P_VOP macroblock */
				mb.mode = MODE_NOT_CODED;
				mb.quant = quant;
				
				//DebugLog<<"mode "<<mb.mode<<endl;
				//DebugLog<<"quant "<<mb.quant<<endl;
				mb.mvs[0].x = mb.mvs[1].x = mb.mvs[2].x = mb.mvs[3].x = 0;
				mb.mvs[0].y = mb.mvs[1].y = mb.mvs[2].y = mb.mvs[3].y = 0;
				mb.field_pred=0; /* (!) */

				block_inter(0, 0, 0);
			//	cp_mb = 0;

				st_mb = x+1;
			}
		}
	}
	LogToFile();
	return 0;
}

int mpeg4analyzer::macroblock_b_vop()
{
	return 0;
}

int mpeg4analyzer::get_cbpy(int intra)
{
	int cbpy;
	uint32_t index = BitstreamShowBits(6);

	BitstreamSkip(cbpy_table[index].len);
	cbpy = cbpy_table[index].code;

	if(!intra) 
		cbpy = 15 - cbpy;

	return cbpy;
}

int mpeg4analyzer::block_intra(int i,Macroblock mb,int acpred_flag,int cbp)
{
	uint16_t data[64],block[64];
	uint32_t iQuant = mb.quant;

	uint32_t iDcScaler = get_dc_scaler(iQuant, i < 4);
    int16_t predictors[8];
    int start_coeff;
   
	if (!acpred_flag) {
		mb.acpred_directions[i] = 0;
	}

	if (quant < intra_dc_threshold) {
		int dc_size;
		int dc_dif;

		dc_size = i < 4 ? get_dc_size_lum() : get_dc_size_chrom();
		
		if( dc_size != 0)
		{
			blockdata.push_back(pair<int,bits>(tellg(),bits(8 - BitstreamNumBitsToByteAlign(),dc_size )));
		}
		
		dc_dif = dc_size ? get_dc_dif(dc_size) : 0;

		if (dc_size > 8) {
			BitstreamSkip(1); /* marker */
		}

		//    block[i * 64 + 0] = dc_dif;
		start_coeff = 1;

		////DebugLog<<"block[0] "<<dc_dif<<" dc_size "<<dc_size<<endl;
		////DebugLog<<"\t addr 0x"<<hex<<tellg()<<dec<<" bit offset "<<8 - BitstreamNumBitsToByteAlign()<<endl;
		
	} else {
		start_coeff = 0;
	}

	if (cbp & (1 << (5 - i))) /* coded */
	{
		int direction = Dec.alternate_vertical_scan ?
			2 : mb.acpred_directions[i];

		get_intra_block(direction, start_coeff);
    }

   /*  we don't decode, we just go through it ... */
	LogToFile();
	return 0;
}

int mpeg4analyzer::block_inter(int cbp,int ref,int bvop)
{
	if( cbp )
	{
		mb_decode(cbp);
	}
	return 0;
}

int mpeg4analyzer::block_inter_field(int cbp,int ref,int bvop)
{

	if( cbp )
	{
		mb_decode(cbp);
	}
	return 0;
}

int mpeg4analyzer::mb_decode( int cbp )
{
	const int direction = Dec.alternate_vertical_scan ? 2 : 0;

	for(int i = 0; i < 6 ; i++ )
	{
		if( cbp & ( 1 << ( 5 - i ) ) )
		{
			get_inter_block(direction);
		}
	}

	return 0;
}

int mpeg4analyzer::get_dc_size_lum()
{

	int code, i;

	code = BitstreamShowBits(11);

	for (i = 11; i > 3; i--) {
		if (code == 1) {
			BitstreamSkip(i);
			return i + 1;
		}
		code >>= 1;
	}

	BitstreamSkip(dc_lum_tab[code].len);
	return dc_lum_tab[code].code;

}


int mpeg4analyzer::get_dc_size_chrom()
{

	uint32_t code, i;

	code = BitstreamShowBits(12);

	for (i = 12; i > 2; i--) {
		if (code == 1) {
			BitstreamSkip(i);
			return i;
		}
		code >>= 1;
	}

	return 3 - BitstreamGetBits(2);

}

int mpeg4analyzer::get_dc_dif(uint32_t dc_size)
{

	int code = BitstreamGetBits(dc_size);
	int msb = code >> (dc_size - 1);

	if (msb == 0)
		return (-1 * (code ^ ((1 << dc_size) - 1)));

	return code;

}

int mpeg4analyzer::get_intra_block(int direction,int coeff)
{
	
	const uint16_t *scan = scan_tables[direction];
	int level, run, last = 0;

	do {
		level = get_coeff(run, last, 1, 0);
		coeff += run;
		if (coeff & ~63) {
			//DebugLog<<"fatal: invalid run or index"<<endl;
			break;
		}

	//	block[scan[coeff]] = level;

		//DebugLog<<"block["<<scan[coeff]<<"] "<<level<<endl;

		if (level < -2047 || level > 2047) {
			//DebugLog<<"warning: intra_overflow "<<level<<endl;
		}
		coeff++;
	} while (!last);

	mpeg4analyzer::LogToFile();
	return 0;
}

int mpeg4analyzer::get_inter_block(int direction)
{
	const uint16_t *scan = scan_tables[direction];
	uint32_t sum = 0;
	int p;
	int level;
	int run;
	int last = 0;

	p=0;
	do{
		level = get_coeff(run,last,0,0);
		p += run;
		if (p & ~63) {
			//DebugLog<<"fatal: invalid run or index"<<endl;
			break;
		}


		//DebugLog<<"block["<<p<<"] "<<level<<endl;
		p++;

	}while(!last);

	LogToFile();
	return 0;
}

#define GET_BITS(cache, n) ((cache)>>(32-(n)))

int mpeg4analyzer::get_coeff(int &run,int &last,int intra,int short_video_header)
{
	
	uint32_t mode;
	int32_t level;
	REVERSE_EVENT *reverse_Event;

	uint32_t cache = BitstreamShowBits(32);
	
	if (short_video_header)		/* inter-VLCs will be used for both intra and inter blocks */
		intra = 0;

	if (GET_BITS(cache, 7) != ESCAPE) {
		reverse_Event = &DCT3D[intra][GET_BITS(cache, 12)];

		if ((level = reverse_Event->Event.level) == 0)
			goto error;

		last = reverse_Event->Event.last;
		run  = reverse_Event->Event.run;

		/* Don't forget to update the bitstream position */
		BitstreamSkip(reverse_Event->len+1);

		return (GET_BITS(cache, reverse_Event->len+1)&0x01) ? -level : level;
	}

	/* flush 7bits of cache */
	cache <<= 7;

	if (short_video_header) {
		/* escape mode 4 - H.263 type, only used if short_video_header = 1  */
		last =  GET_BITS(cache, 1);
		run  = (GET_BITS(cache, 7) &0x3f);
		level = (GET_BITS(cache, 15)&0xff);

		if (level == 0 || level == 128)
			//DebugLog<<"Illegal LEVEL for ESCAPE mode 4: "<<level<<endl;

		/* We've "eaten" 22 bits */
		BitstreamSkip(22);

		return (level << 24) >> 24;
	}

	if ((mode = GET_BITS(cache, 2)) < 3) {
		const int skip[3] = {1, 1, 2};
		cache <<= skip[mode];

		reverse_Event = &DCT3D[intra][GET_BITS(cache, 12)];

		if ((level = reverse_Event->Event.level) == 0)
			goto error;

		last = reverse_Event->Event.last;
		run  = reverse_Event->Event.run;

		if (mode < 2) {
			/* first escape mode, level is offset */
			level += max_level[intra][last][run];
		} else {
			/* second escape mode, run is offset */
			run += max_run[intra][last][level] + 1;
		}
		
		/* Update bitstream position */
		BitstreamSkip(7 + skip[mode] + reverse_Event->len + 1);

		return (GET_BITS(cache, reverse_Event->len+1)&0x01) ? -level : level;
	}

	/* third escape mode - fixed length codes */
	cache <<= 2;
	last =  GET_BITS(cache, 1);
	run  = (GET_BITS(cache, 7)&0x3f);
	level = (GET_BITS(cache, 20)&0xfff);
	
	/* Update bitstream position */
	BitstreamSkip(30);

	return (level << 20) >> 20;

  error:
	run = 64;
	return NULL;
}

void mpeg4analyzer::init_vlc_tables(void)
{
	uint32_t i, j, k, intra, last, run,  run_esc, level, level_esc, escape, escape_len, offset;
	int32_t l;

	for (intra = 0; intra < 2; intra++)
		for (i = 0; i < 4096; i++)
			DCT3D[intra][i].Event.level = 0;

	for (intra = 0; intra < 2; intra++) {
		for (last = 0; last < 2; last++) {
			for (run = 0; run < 63 + last; run++) {
				for (level = 0; level < (uint32_t)(32 << intra); level++) {
					offset = !intra * LEVELOFFSET;
					coeff_VLC[intra][last][level + offset][run].len = 128;
				}
			}
		}
	}

	for (intra = 0; intra < 2; intra++) {
		for (i = 0; i < 102; i++) {
			offset = !intra * LEVELOFFSET;

			for (j = 0; j < (uint32_t)(1 << (12 - coeff_tab[intra][i].vlc.len)); j++) {
				DCT3D[intra][(coeff_tab[intra][i].vlc.code << (12 - coeff_tab[intra][i].vlc.len)) | j].len	 = coeff_tab[intra][i].vlc.len;
				DCT3D[intra][(coeff_tab[intra][i].vlc.code << (12 - coeff_tab[intra][i].vlc.len)) | j].Event = coeff_tab[intra][i].Event;
			}

			coeff_VLC[intra][coeff_tab[intra][i].Event.last][coeff_tab[intra][i].Event.level + offset][coeff_tab[intra][i].Event.run].code
				= coeff_tab[intra][i].vlc.code << 1;
			coeff_VLC[intra][coeff_tab[intra][i].Event.last][coeff_tab[intra][i].Event.level + offset][coeff_tab[intra][i].Event.run].len
				= coeff_tab[intra][i].vlc.len + 1;

			if (!intra) {
				coeff_VLC[intra][coeff_tab[intra][i].Event.last][offset - coeff_tab[intra][i].Event.level][coeff_tab[intra][i].Event.run].code
					= (coeff_tab[intra][i].vlc.code << 1) | 1;
				coeff_VLC[intra][coeff_tab[intra][i].Event.last][offset - coeff_tab[intra][i].Event.level][coeff_tab[intra][i].Event.run].len
					= coeff_tab[intra][i].vlc.len + 1;
			}
		}
	}

	for (intra = 0; intra < 2; intra++) {
		for (last = 0; last < 2; last++) {
			for (run = 0; run < 63 + last; run++) {
				for (level = 1; level < (uint32_t)(32 << intra); level++) {

					if (level <= max_level[intra][last][run] && run <= max_run[intra][last][level])
					    continue;

					offset = !intra * LEVELOFFSET;
                    level_esc = level - max_level[intra][last][run];
					run_esc = run - 1 - max_run[intra][last][level];

					if (level_esc <= max_level[intra][last][run] && run <= max_run[intra][last][level_esc]) {
						escape     = ESCAPE1;
						escape_len = 7 + 1;
						run_esc    = run;
					} else {
						if (run_esc <= max_run[intra][last][level] && level <= max_level[intra][last][run_esc]) {
							escape     = ESCAPE2;
							escape_len = 7 + 2;
							level_esc  = level;
						} else {
							if (!intra) {
								coeff_VLC[intra][last][level + offset][run].code
									= (ESCAPE3 << 21) | (last << 20) | (run << 14) | (1 << 13) | ((level & 0xfff) << 1) | 1;
								coeff_VLC[intra][last][level + offset][run].len = 30;
									coeff_VLC[intra][last][offset - level][run].code
									= (ESCAPE3 << 21) | (last << 20) | (run << 14) | (1 << 13) | ((-(int32_t)level & 0xfff) << 1) | 1;
								coeff_VLC[intra][last][offset - level][run].len = 30;
							}
							continue;
						}
					}

					coeff_VLC[intra][last][level + offset][run].code
						= (escape << coeff_VLC[intra][last][level_esc + offset][run_esc].len)
						|  coeff_VLC[intra][last][level_esc + offset][run_esc].code;
					coeff_VLC[intra][last][level + offset][run].len
						= coeff_VLC[intra][last][level_esc + offset][run_esc].len + escape_len;

					if (!intra) {
						coeff_VLC[intra][last][offset - level][run].code
							= (escape << coeff_VLC[intra][last][level_esc + offset][run_esc].len)
							|  coeff_VLC[intra][last][level_esc + offset][run_esc].code | 1;
						coeff_VLC[intra][last][offset - level][run].len
							= coeff_VLC[intra][last][level_esc + offset][run_esc].len + escape_len;
					}
				}

				if (!intra) {
					coeff_VLC[intra][last][0][run].code
						= (ESCAPE3 << 21) | (last << 20) | (run << 14) | (1 << 13) | ((-32 & 0xfff) << 1) | 1;
					coeff_VLC[intra][last][0][run].len = 30;
				}
			}
		}
	}

	/* init sprite_trajectory tables
	 * even if GMC is not specified (it might be used later...) */
/*
	sprite_trajectory_code[0+16384].code = 0;
	sprite_trajectory_code[0+16384].len = 0;
	for (k=0;k<14;k++) {
		int limit = (1<<k);

		for (l=-(2*limit-1); l <= -limit; l++) {
			sprite_trajectory_code[l+16384].code = (2*limit-1)+l;
			sprite_trajectory_code[l+16384].len = k+1;
		}

		for (l=limit; l<= 2*limit-1; l++) {
			sprite_trajectory_code[l+16384].code = l;
			sprite_trajectory_code[l+16384].len = k+1;
		}
	}*/
}

int mpeg4analyzer::get_motion_vector()
{
	const int scale_fac = 1 << (fcode_forward - 1);
	const int high = (32 * scale_fac) - 1;
	const int low = ((-32) * scale_fac);
	const int range = (64 * scale_fac);

	get_mv();
	get_mv();

	return 0;
}

int mpeg4analyzer::get_motion_vector_interlaced(Macroblock mb)
{
	const int scale_fac = 1 << (fcode_forward - 1);
	const int high = (32 * scale_fac) - 1;
	const int low = ((-32) * scale_fac);
	const int range = (64 * scale_fac);

	if( ! mb.field_pred )
	{
		get_mv();
		get_mv();
	}
	else
	{
		get_mv();
		get_mv();
		get_mv();
		get_mv();
	}
	return 0;
}

int mpeg4analyzer::get_mv()
{
	int res,mv,scale_fac = 1 << (fcode_forward - 1);
	int data ;

	uint32_t index;

	if( ! BitstreamGetBits(1) )
	{
		index = BitstreamShowBits(12);

		if (index >= 512)
		{
			index = (index >> 8) - 2;
			BitstreamSkip(TMNMVtab0[index].len);
			data = TMNMVtab0[index].code;
			goto finish;
		}

		if (index >= 128) 
		{
			index = (index >> 2) - 32;
			BitstreamSkip(TMNMVtab1[index].len);
			data = TMNMVtab1[index].code;
			goto finish;
		}

		index -= 4;

		BitstreamSkip(TMNMVtab2[index].len);
		data = TMNMVtab2[index].code;

	}
	else
	{
		data = 0;
	}

finish:

	if( scale_fac == 1 || data == 0 )
		return data;

	res = BitstreamGetBits( fcode_forward - 1);
	mv = ((abs(data) - 1) * scale_fac) + res + 1;

	return data < 0 ? -mv : mv;

}

int mpeg4analyzer::mbgmc(int cbp)
{

	if( cbp )
	{
		mb_decode(cbp);
	}

	return 0;
}