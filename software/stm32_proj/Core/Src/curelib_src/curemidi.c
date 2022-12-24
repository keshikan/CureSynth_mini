/*
 *******************************************************************************
 *  [curemidi.c]
 *  This module is for analyzing MIDI messages and controlling "curesynth.h/c"
 *
 *******************************************************************************
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */


#include "curelib_inc/curemidi.h"

#include "./curelib_inc/patchlist.h"
#include "./curelib_inc/curedbg.h"

#include <string.h>

#include "cmsis_gcc.h"


/////const variables/////
extern const MidiPatch patch_melody[128];
extern const MidiPatch patch_drum[128];

////private variables////
MidiAnalysisStatus analyzed_status[MIDI_INPUT_SRC_NUM];
MIDIEvent midi_event[MIDI_INPUT_SRC_NUM];	//received midi data
DispSettingChangeInfo midi_ui_setting_info;


//japanese strings
char string_ref_upper[DISPSETTING_STRING_MAX_LENGTH] = {'\0',};
char string_ref_downer[DISPSETTING_STRING_MAX_LENGTH] = {'\0',};
uint8_t string_upper_ptr = 0;
uint8_t string_downer_ptr = 0;

const float CentTable[CENT_TABLE_NUM] = {1.000000, 1.000578, 1.001156, 1.001734, 1.002313, 1.002892, 1.003472, 1.004052, 1.004632, 1.005212, 1.005793, 1.006374, 1.006956, 1.007537, 1.008120, 1.008702, 1.009285, 1.009868, 1.010451, 1.011035, 1.011619, 1.012204, 1.012789, 1.013374, 1.013959, 1.014545, 1.015132, 1.015718, 1.016305, 1.016892, 1.017480, 1.018068, 1.018656, 1.019244, 1.019833, 1.020423, 1.021012, 1.021602, 1.022192, 1.022783, 1.023374, 1.023965, 1.024557, 1.025149, 1.025741, 1.026334, 1.026927, 1.027520, 1.028114, 1.028708, 1.029302, 1.029897, 1.030492, 1.031087, 1.031683, 1.032279, 1.032876, 1.033472, 1.034070, 1.034667, 1.035265, 1.035863, 1.036462, 1.037060, 1.037660, 1.038259, 1.038859, 1.039459, 1.040060, 1.040661, 1.041262, 1.041864, 1.042466, 1.043068, 1.043671, 1.044274, 1.044877, 1.045481, 1.046085, 1.046689, 1.047294, 1.047899, 1.048505, 1.049111, 1.049717, 1.050323, 1.050930, 1.051537, 1.052145, 1.052753, 1.053361, 1.053970, 1.054579, 1.055188, 1.055798, 1.056408, 1.057018, 1.057629, 1.058240, 1.058851, 1.059463, 1.060075, 1.060688, 1.061301, 1.061914, 1.062527, 1.063141, 1.063756, 1.064370, 1.064985, 1.065601, 1.066216, 1.066832, 1.067449, 1.068065, 1.068683, 1.069300, 1.069918, 1.070536, 1.071155, 1.071773, 1.072393, 1.073012, 1.073632, 1.074253, 1.074873, 1.075494, 1.076116, 1.076738, 1.077360, 1.077982, 1.078605, 1.079228, 1.079852, 1.080476, 1.081100, 1.081725, 1.082350, 1.082975, 1.083601, 1.084227, 1.084853, 1.085480, 1.086107, 1.086735, 1.087363, 1.087991, 1.088620, 1.089249, 1.089878, 1.090508, 1.091138, 1.091768, 1.092399, 1.093030, 1.093662, 1.094294, 1.094926, 1.095559, 1.096192, 1.096825, 1.097459, 1.098093, 1.098727, 1.099362, 1.099997, 1.100633, 1.101269, 1.101905, 1.102542, 1.103179, 1.103816, 1.104454, 1.105092, 1.105731, 1.106370, 1.107009, 1.107648, 1.108288, 1.108929, 1.109569, 1.110211, 1.110852, 1.111494, 1.112136, 1.112779, 1.113422, 1.114065, 1.114709, 1.115353, 1.115997, 1.116642, 1.117287, 1.117933, 1.118579, 1.119225, 1.119872, 1.120519, 1.121166, 1.121814, 1.122462, 1.123111, 1.123760, 1.124409, 1.125058, 1.125709, 1.126359, 1.127010, 1.127661, 1.128312, 1.128964, 1.129617, 1.130269, 1.130922, 1.131576, 1.132230, 1.132884, 1.133538, 1.134193, 1.134849, 1.135504, 1.136161, 1.136817, 1.137474, 1.138131, 1.138789, 1.139447, 1.140105, 1.140764, 1.141423, 1.142082, 1.142742, 1.143402, 1.144063, 1.144724, 1.145386, 1.146047, 1.146710, 1.147372, 1.148035, 1.148698, 1.149362, 1.150026, 1.150691, 1.151355, 1.152021, 1.152686, 1.153352, 1.154019, 1.154686, 1.155353, 1.156020, 1.156688, 1.157357, 1.158025, 1.158694, 1.159364, 1.160034, 1.160704, 1.161375, 1.162046, 1.162717, 1.163389, 1.164061, 1.164734, 1.165407, 1.166080, 1.166754, 1.167428, 1.168102, 1.168777, 1.169453, 1.170128, 1.170804, 1.171481, 1.172158, 1.172835, 1.173513, 1.174191, 1.174869, 1.175548, 1.176227, 1.176907, 1.177587, 1.178267, 1.178948, 1.179629, 1.180311, 1.180993, 1.181675, 1.182358, 1.183041, 1.183724, 1.184408, 1.185093, 1.185778, 1.186463, 1.187148, 1.187834, 1.188520, 1.189207, 1.189894, 1.190582, 1.191270, 1.191958, 1.192647, 1.193336, 1.194025, 1.194715, 1.195405, 1.196096, 1.196787, 1.197479, 1.198171, 1.198863, 1.199556, 1.200249, 1.200942, 1.201636, 1.202330, 1.203025, 1.203720, 1.204416, 1.205112, 1.205808, 1.206505, 1.207202, 1.207899, 1.208597, 1.209295, 1.209994, 1.210693, 1.211393, 1.212093, 1.212793, 1.213494, 1.214195, 1.214896, 1.215598, 1.216301, 1.217004, 1.217707, 1.218410, 1.219114, 1.219819, 1.220523, 1.221229, 1.221934, 1.222640, 1.223347, 1.224054, 1.224761, 1.225468, 1.226177, 1.226885, 1.227594, 1.228303, 1.229013, 1.229723, 1.230433, 1.231144, 1.231856, 1.232568, 1.233280, 1.233992, 1.234705, 1.235419, 1.236132, 1.236847, 1.237561, 1.238276, 1.238992, 1.239708, 1.240424, 1.241141, 1.241858, 1.242575, 1.243293, 1.244012, 1.244730, 1.245450, 1.246169, 1.246889, 1.247610, 1.248331, 1.249052, 1.249774, 1.250496, 1.251218, 1.251941, 1.252664, 1.253388, 1.254112, 1.254837, 1.255562, 1.256288, 1.257013, 1.257740, 1.258466, 1.259194, 1.259921, 1.260649, 1.261377, 1.262106, 1.262835, 1.263565, 1.264295, 1.265026, 1.265757, 1.266488, 1.267220, 1.267952, 1.268684, 1.269418, 1.270151, 1.270885, 1.271619, 1.272354, 1.273089, 1.273825, 1.274561, 1.275297, 1.276034, 1.276771, 1.277509, 1.278247, 1.278986, 1.279725, 1.280464, 1.281204, 1.281944, 1.282685, 1.283426, 1.284167, 1.284909, 1.285652, 1.286395, 1.287138, 1.287882, 1.288626, 1.289370, 1.290115, 1.290861, 1.291607, 1.292353, 1.293100, 1.293847, 1.294594, 1.295342, 1.296091, 1.296840, 1.297589, 1.298339, 1.299089, 1.299839, 1.300590, 1.301342, 1.302094, 1.302846, 1.303599, 1.304352, 1.305106, 1.305860, 1.306614, 1.307369, 1.308125, 1.308880, 1.309637, 1.310393, 1.311151, 1.311908, 1.312666, 1.313425, 1.314183, 1.314943, 1.315703, 1.316463, 1.317223, 1.317984, 1.318746, 1.319508, 1.320270, 1.321033, 1.321796, 1.322560, 1.323324, 1.324089, 1.324854, 1.325619, 1.326385, 1.327152, 1.327919, 1.328686, 1.329454, 1.330222, 1.330990, 1.331759, 1.332529, 1.333299, 1.334069, 1.334840, 1.335611, 1.336383, 1.337155, 1.337928, 1.338701, 1.339474, 1.340248, 1.341022, 1.341797, 1.342573, 1.343348, 1.344124, 1.344901, 1.345678, 1.346456, 1.347234, 1.348012, 1.348791, 1.349570, 1.350350, 1.351130, 1.351911, 1.352692, 1.353474, 1.354256, 1.355038, 1.355821, 1.356604, 1.357388, 1.358172, 1.358957, 1.359742, 1.360528, 1.361314, 1.362101, 1.362888, 1.363675, 1.364463, 1.365251, 1.366040, 1.366830, 1.367619, 1.368409, 1.369200, 1.369991, 1.370783, 1.371575, 1.372367, 1.373160, 1.373954, 1.374748, 1.375542, 1.376337, 1.377132, 1.377928, 1.378724, 1.379520, 1.380317, 1.381115, 1.381913, 1.382711, 1.383510, 1.384310, 1.385109, 1.385910, 1.386711, 1.387512, 1.388313, 1.389116, 1.389918, 1.390721, 1.391525, 1.392329, 1.393133, 1.393938, 1.394744, 1.395550, 1.396356, 1.397163, 1.397970, 1.398778, 1.399586, 1.400395, 1.401204, 1.402013, 1.402823, 1.403634, 1.404445, 1.405256, 1.406068, 1.406881, 1.407694, 1.408507, 1.409321, 1.410135, 1.410950, 1.411765, 1.412581, 1.413397, 1.414214, 1.415031, 1.415848, 1.416666, 1.417485, 1.418304, 1.419123, 1.419943, 1.420764, 1.421585, 1.422406, 1.423228, 1.424050, 1.424873, 1.425696, 1.426520, 1.427344, 1.428169, 1.428994, 1.429820, 1.430646, 1.431473, 1.432300, 1.433127, 1.433955, 1.434784, 1.435613, 1.436442, 1.437272, 1.438103, 1.438934, 1.439765, 1.440597, 1.441429, 1.442262, 1.443095, 1.443929, 1.444763, 1.445598, 1.446434, 1.447269, 1.448105, 1.448942, 1.449779, 1.450617, 1.451455, 1.452294, 1.453133, 1.453973, 1.454813, 1.455653, 1.456494, 1.457336, 1.458178, 1.459020, 1.459863, 1.460707, 1.461551, 1.462395, 1.463240, 1.464086, 1.464932, 1.465778, 1.466625, 1.467472, 1.468320, 1.469169, 1.470018, 1.470867, 1.471717, 1.472567, 1.473418, 1.474269, 1.475121, 1.475973, 1.476826, 1.477679, 1.478533, 1.479388, 1.480242, 1.481098, 1.481953, 1.482810, 1.483666, 1.484524, 1.485381, 1.486240, 1.487098, 1.487958, 1.488817, 1.489677, 1.490538, 1.491399, 1.492261, 1.493123, 1.493986, 1.494849, 1.495713, 1.496577, 1.497442, 1.498307, 1.499173, 1.500039, 1.500906, 1.501773, 1.502641, 1.503509, 1.504378, 1.505247, 1.506116, 1.506987, 1.507857, 1.508729, 1.509600, 1.510473, 1.511345, 1.512219, 1.513092, 1.513967, 1.514841, 1.515717, 1.516592, 1.517469, 1.518345, 1.519223, 1.520100, 1.520979, 1.521858, 1.522737, 1.523617, 1.524497, 1.525378, 1.526259, 1.527141, 1.528023, 1.528906, 1.529790, 1.530674, 1.531558, 1.532443, 1.533328, 1.534214, 1.535101, 1.535988, 1.536875, 1.537763, 1.538652, 1.539541, 1.540430, 1.541320, 1.542211, 1.543102, 1.543993, 1.544886, 1.545778, 1.546671, 1.547565, 1.548459, 1.549354, 1.550249, 1.551145, 1.552041, 1.552938, 1.553835, 1.554733, 1.555631, 1.556530, 1.557429, 1.558329, 1.559230, 1.560130, 1.561032, 1.561934, 1.562836, 1.563739, 1.564643, 1.565547, 1.566451, 1.567356, 1.568262, 1.569168, 1.570075, 1.570982, 1.571890, 1.572798, 1.573707, 1.574616, 1.575526, 1.576436, 1.577347, 1.578258, 1.579170, 1.580083, 1.580996, 1.581909, 1.582823, 1.583738, 1.584653, 1.585568, 1.586484, 1.587401, 1.588318, 1.589236, 1.590154, 1.591073, 1.591992, 1.592912, 1.593832, 1.594753, 1.595675, 1.596597, 1.597519, 1.598442, 1.599366, 1.600290, 1.601215, 1.602140, 1.603065, 1.603992, 1.604918, 1.605846, 1.606774, 1.607702, 1.608631, 1.609560, 1.610490, 1.611421, 1.612352, 1.613284, 1.614216, 1.615148, 1.616082, 1.617015, 1.617950, 1.618884, 1.619820, 1.620756, 1.621692, 1.622629, 1.623567, 1.624505, 1.625443, 1.626383, 1.627322, 1.628263, 1.629203, 1.630145, 1.631087, 1.632029, 1.632972, 1.633915, 1.634860, 1.635804, 1.636749, 1.637695, 1.638641, 1.639588, 1.640535, 1.641483, 1.642432, 1.643381, 1.644330, 1.645280, 1.646231, 1.647182, 1.648134, 1.649086, 1.650039, 1.650992, 1.651946, 1.652901, 1.653856, 1.654811, 1.655767, 1.656724, 1.657681, 1.658639, 1.659597, 1.660556, 1.661516, 1.662476, 1.663436, 1.664397, 1.665359, 1.666321, 1.667284, 1.668247, 1.669211, 1.670176, 1.671141, 1.672106, 1.673073, 1.674039, 1.675006, 1.675974, 1.676943, 1.677912, 1.678881, 1.679851, 1.680822, 1.681793, 1.682765, 1.683737, 1.684710, 1.685683, 1.686657, 1.687632, 1.688607, 1.689582, 1.690559, 1.691535, 1.692513, 1.693491, 1.694469, 1.695448, 1.696428, 1.697408, 1.698389, 1.699370, 1.700352, 1.701334, 1.702317, 1.703301, 1.704285, 1.705270, 1.706255, 1.707241, 1.708227, 1.709214, 1.710202, 1.711190, 1.712179, 1.713168, 1.714158, 1.715148, 1.716139, 1.717131, 1.718123, 1.719116, 1.720109, 1.721103, 1.722097, 1.723092, 1.724088, 1.725084, 1.726081, 1.727078, 1.728076, 1.729074, 1.730074, 1.731073, 1.732073, 1.733074, 1.734075, 1.735077, 1.736080, 1.737083, 1.738087, 1.739091, 1.740096, 1.741101, 1.742107, 1.743114, 1.744121, 1.745129, 1.746137, 1.747146, 1.748155, 1.749165, 1.750176, 1.751187, 1.752199, 1.753211, 1.754224, 1.755238, 1.756252, 1.757267, 1.758282, 1.759298, 1.760315, 1.761332, 1.762349, 1.763368, 1.764387, 1.765406, 1.766426, 1.767447, 1.768468, 1.769490, 1.770512, 1.771535, 1.772559, 1.773583, 1.774608, 1.775633, 1.776659, 1.777685, 1.778712, 1.779740, 1.780769, 1.781797, 1.782827, 1.783857, 1.784888, 1.785919, 1.786951, 1.787983, 1.789016, 1.790050, 1.791084, 1.792119, 1.793155, 1.794191, 1.795227, 1.796265, 1.797303, 1.798341, 1.799380, 1.800420, 1.801460, 1.802501, 1.803542, 1.804584, 1.805627, 1.806670, 1.807714, 1.808759, 1.809804, 1.810850, 1.811896, 1.812943, 1.813990, 1.815038, 1.816087, 1.817136, 1.818186, 1.819237, 1.820288, 1.821340, 1.822392, 1.823445, 1.824499, 1.825553, 1.826608, 1.827663, 1.828719, 1.829776, 1.830833, 1.831891, 1.832949, 1.834008, 1.835068, 1.836128, 1.837189, 1.838250, 1.839313, 1.840375, 1.841439, 1.842503, 1.843567, 1.844632, 1.845698, 1.846765, 1.847832, 1.848899, 1.849968, 1.851036, 1.852106, 1.853176, 1.854247, 1.855318, 1.856390, 1.857463, 1.858536, 1.859610, 1.860684, 1.861759, 1.862835, 1.863911, 1.864988, 1.866066, 1.867144, 1.868223, 1.869302, 1.870382, 1.871463, 1.872544, 1.873626, 1.874709, 1.875792, 1.876876, 1.877960, 1.879045, 1.880131, 1.881218, 1.882304, 1.883392, 1.884480, 1.885569, 1.886659, 1.887749, 1.888839, 1.889931, 1.891023, 1.892115, 1.893209, 1.894302, 1.895397, 1.896492, 1.897588, 1.898684, 1.899781, 1.900879, 1.901977, 1.903076, 1.904176, 1.905276, 1.906377, 1.907478, 1.908580, 1.909683, 1.910787, 1.911891, 1.912995, 1.914101, 1.915207, 1.916313, 1.917420, 1.918528, 1.919637, 1.920746, 1.921856, 1.922966, 1.924077, 1.925189, 1.926301, 1.927414, 1.928528, 1.929642, 1.930757, 1.931873, 1.932989, 1.934106, 1.935223, 1.936341, 1.937460, 1.938580, 1.939700, 1.940820, 1.941942, 1.943064, 1.944187, 1.945310, 1.946434, 1.947559, 1.948684, 1.949810, 1.950936, 1.952064, 1.953191, 1.954320, 1.955449, 1.956579, 1.957709, 1.958841, 1.959972, 1.961105, 1.962238, 1.963372, 1.964506, 1.965641, 1.966777, 1.967913, 1.969050, 1.970188, 1.971326, 1.972465, 1.973605, 1.974745, 1.975886, 1.977028, 1.978170, 1.979313, 1.980457, 1.981601, 1.982746, 1.983892, 1.985038, 1.986185, 1.987333, 1.988481, 1.989630, 1.990779, 1.991930, 1.993081, 1.994232, 1.995384, 1.996537, 1.997691, 1.998845};

////public variables////
MidiMasterChannel master;
RingBufferU8 rxbuf;
uint8_t rxbuf_buffer[CUREMIDI_BUFFER_LENGTH];
bool is_debug_displaying = false;

////private func////
void cureMidiSetProgramChange(uint8_t ch, uint8_t pgno);
void cureMidiSetNoteOn(uint8_t ch, uint8_t notenum, uint8_t vel);
void cureMidiSetNoteOff(uint8_t ch, uint8_t val);
void cureMidiSetNoteOffImmediately(uint8_t ch, uint8_t val);

void cureMidiMasterChannelInit(MidiMasterChannel* mtr);


//extern midi message reading timer
extern TIM_HandleTypeDef htim16;

//void cureMidiMain()
//{
//	while( BUFFER_SUCCESS == cureMidiBufferDequeue() ){
//
//		if( cureMidiEventIsGenerated() ){// Generate MIDI event from UART buffer.
//			//Analyze MIDI Message.
//			cureMidiAnalyzeEvent();
//		}
//	}
//}


FUNC_STATUS cureMidiInit()
{
	if( BUFFER_FAILURE == cureRingBufferU8Init(&rxbuf, CUREMIDI_BUFFER_LENGTH, rxbuf_buffer) ){
		return FUNC_ERROR;
	}

	cureMidiMasterChannelInit(&master);
	dpChangeInfoInit(&midi_ui_setting_info);

	if ( FUNC_ERROR == cureSynthInit()){
		return FUNC_ERROR;
	}

	//Program change init
	for(uint32_t i=0; i<CHNUM; i++){
		cureMidiSetProgramChange(i, 0x00);
	}

	return FUNC_SUCCESS;

}

void cureMidiChannelInit(MidiChannel* ch)
{
	uint8_t i;

	for(i=0; i<128; i++){
		ch->operator_to_note.stat[i] = OPERATOR_OFF;
		ch->hold_note_list[i] = false;
		ch->sostenuto_note_list[i] = STN_INIT;
	}

	for(i=0; i<OPNUM; i++){
		ch->operator_to_channel.ison[i] = false;
	}

	cureSynthOperatorInit( &(ch->operatorsettings) );

	ch->operatorsettings.wav.type = SAW;
	ch->operatorsettings.ringmod.multiply = 0.5;
	ch->operatorsettings.ringmod.gain = 100;
	ch->operatorsettings.env.release = 10;
	ch->operatorsettings.env.attack = 1;


	ch->isHoldOn = false;
	ch->isSostenutoOn = false;

	ch->rpn_lsb = 255;
	ch->rpn_msb = 255;
	ch->rpn_type = RPN_NULL;
	ch->nrpn_lsb = 255;
	ch->nrpn_msb = 255;
	ch->nrpn_type = NRPN_NULL;
	ch->rpn_nrpn_choose = CHOSE_RPN;

	ch->fine_tune_msb = 0x40;
	ch->fine_tune_lsb = 0x00;
	ch->coarse_tune_msb = 0x40;
	ch->tuning_scalefactor = 1.0f;


}

void cureMidiMasterChannelInit(MidiMasterChannel* mtr)
{
	uint8_t i;

	mtr->front_opnum = 0;
	for(i=0; i<CHNUM; i++){
		cureMidiChannelInit(&(mtr->channel[i]));
	}
}


//TODO: use function macros.
//BUFFER_STATUS cureMidiBufferEnqueue(uint8_t* inputc)
//{
//	return cureRingBufferU8Enqueue(&rxbuf, inputc);
//}
//
//BUFFER_STATUS cureMidiBufferDequeue()
//{
//	return cureRingBufferU8Dequeue(&rxbuf, &midi_buf);
//}

//bool cureMidiBufferIsEmpty()
//{
//	if( 0 == _cureRingBufferU8GetUsedSize(&rxbuf) ){
//		return true;
//	}
//
//	return false;
//}

bool cureMidiEventIsGenerated(uint8_t midi_buf, uint8_t src_no)
{
	uint8_t upper_half_byte= (midi_buf) & 0xF0;
	uint8_t lower_half_byte= (midi_buf) & 0x0F;

	if( upper_half_byte & 0x80 ){// status byte.
		if( 0xF0 == upper_half_byte ){// MIDI System Message

			switch(lower_half_byte){

			case 0x00://SysEx Start
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_SYSEX;
				analyzed_status[src_no].data_idx = 0;
				analyzed_status[src_no].stat = WAIT_SYSTEM_DATA;
				break;

			case 0x07://SysEx End

				if(WAIT_SYSTEM_DATA == analyzed_status[src_no].stat){

					analyzed_status[src_no].stat = END_ANALYSIS;

				}else{//start SysEx at F7. ignore all data. TODO
					midi_event[src_no].type = analyzed_status[src_no].type = MSG_SYSEX;
					analyzed_status[src_no].data_idx = 0;
					analyzed_status[src_no].stat = IGNORE_DATA;
				}
				break;

			default://Other System messages (e.g. 0xFE(active sensing))
				//ignore all
				return false;

			}

		}else{// MIDI Channel Message.

			switch(upper_half_byte){

			case 0x90://Note On Message.
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_NOTE_ON;
				analyzed_status[src_no].stat = WAIT_DATA1;
				midi_event[src_no].channel = lower_half_byte;
				analyzed_status[src_no].channel = lower_half_byte;
				break;

			case 0x80://Note Off Message.
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_NOTE_OFF;
				analyzed_status[src_no].stat = WAIT_DATA1;
				midi_event[src_no].channel = lower_half_byte;
				analyzed_status[src_no].channel = lower_half_byte;
				break;

			case 0xE0://Pitch Bend.
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_PITCH;
				analyzed_status[src_no].stat = WAIT_DATA1;
				midi_event[src_no].channel = lower_half_byte;
				analyzed_status[src_no].channel = lower_half_byte;
				break;

			case 0xB0://Control Change
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_CC;
				analyzed_status[src_no].stat = WAIT_DATA1;
				midi_event[src_no].channel = lower_half_byte;
				analyzed_status[src_no].channel = lower_half_byte;
				break;

			case 0xC0://Program Change
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_PROG;
				analyzed_status[src_no].stat = WAIT_DATA1;
				midi_event[src_no].channel = lower_half_byte;
				analyzed_status[src_no].channel = lower_half_byte;
				break;

			default:
				midi_event[src_no].type = analyzed_status[src_no].type = MSG_NOTHING;
				analyzed_status[src_no].stat = START_ANALYSIS;
				break;
			}
		}
	}else{//data byte
		switch(analyzed_status[src_no].stat){

		case WAIT_DATA1:
			midi_event[src_no].data_byte[0] = (midi_buf);
			if(MSG_NOTE_ON == analyzed_status[src_no].type || MSG_NOTE_OFF == analyzed_status[src_no].type || MSG_PITCH  == analyzed_status[src_no].type || MSG_CC == analyzed_status[src_no].type ){
				analyzed_status[src_no].stat = WAIT_DATA2;
			}else if( MSG_PROG == analyzed_status[src_no].type ){
				analyzed_status[src_no].stat = END_ANALYSIS;
			}else{
				analyzed_status[src_no].stat = START_ANALYSIS;
			}
			break;

		case WAIT_DATA2:
			midi_event[src_no].data_byte[1] = (midi_buf);
			analyzed_status[src_no].stat = END_ANALYSIS;
//delete
//			//When 2nd data_byte is 0x00 in note-on message, perform note off.
//			if(MSG_NOTE_ON == analyzed_status[src_no].type && 0x00 == midi_buf){
//				midi_event[src_no].type = analyzed_status[src_no].type = MSG_NOTE_OFF;
//			}
			break;

		case WAIT_SYSTEM_DATA:
			midi_event[src_no].data_byte[analyzed_status[src_no].data_idx++] = (midi_buf);

			if(analyzed_status[src_no].data_idx > (MIDI_DATABYTE_MAX - 1) ){
//				analyzed_status[src_no].stat = END_ANALYSIS;
				analyzed_status[src_no].data_idx = (MIDI_DATABYTE_MAX - 1);//ignore, very long system exclusive.
			}
			break;

		case END_ANALYSIS://running status
			midi_event[src_no].data_byte[0] = (midi_buf);
			analyzed_status[src_no].stat = WAIT_DATA2;
			break;

		case START_ANALYSIS:
			break;

		case IGNORE_DATA:
			break;

		default:
			break;
		}
	}

	if(END_ANALYSIS == analyzed_status[src_no].stat){
		return true;
	}else{
		return false;
	}

}

float cureMidiRingmodMultofloat(uint8_t dat_int)
{
	float ret;

	if(dat_int != 0){
		ret = ( (float)(dat_int & 0x7F) + 1.0f ) / 32.0f;
	}else{
		ret = ( 1.0f / 512.0f );
	}

	return ret;
}

void cureMidiSetDrumParam(uint8_t ch, uint8_t notenum)
{
	master.channel[ch].operatorsettings.wav.type 				= patch_drum[ notenum ].wtype;
	master.channel[ch].operatorsettings.ntype 					= patch_drum[ notenum ].ntype;
	master.channel[ch].operatorsettings.wavenum				    = patch_drum[ notenum ].wavenum;
	master.channel[ch].operatorsettings.ringmod.multiply 		=cureMidiRingmodMultofloat(patch_drum[ notenum ].ringmod_multiply);
	master.channel[ch].operatorsettings.ringmod.gain 			= patch_drum[ notenum ].ringmod_gain;
	master.channel[ch].operatorsettings.wav.pitch_sweep_type 	= patch_drum[ notenum ].pitch_sweep_type;
	master.channel[ch].operatorsettings.wav.pitch_sweep_spd 	= patch_drum[ notenum ].pitch_sweep_spd;
	master.channel[ch].operatorsettings.env.attack 				= patch_drum[ notenum ].attack * 2;
	master.channel[ch].operatorsettings.env.decay 				= patch_drum[ notenum ].decay * 2;
	master.channel[ch].operatorsettings.env.sustainLevel 		= patch_drum[ notenum ].sustainLevel * 2;
	master.channel[ch].operatorsettings.env.sustainRate 		= patch_drum[ notenum ].sustainRate * 2;
	master.channel[ch].operatorsettings.env.release 			= patch_drum[ notenum ].release * 2;
	master.channel[ch].operatorsettings.out_gain	 			= patch_drum[ notenum ].out_gain;

	cureSynthOperatorCopy(&master.channel[ch].operatorsettings, &au_operator[master.front_opnum]);

}

float _cureMidiConvCentToMul(int16_t cent)
{
	float ret;

	if(cent >= 0){
		ret = CentTable[cent];
	}else{
		ret = CentTable[CENT_TABLE_NUM - abs(cent)] - 1.0f;
	}

	return ret;
}

void _cureMidiUnisonOn(int16_t list[UNISON_MAX_NUM], uint8_t ch, uint8_t note, uint8_t vel)
{
//	uint16_t idx_list = 0;
	static int8_t sign = 1;

	for(uint32_t i=0; i<UNISON_MAX_NUM; i++){
		if(list[i] < 0){
			break;
		}
		cureSynthOperatorCopy(&master.channel[ch].operatorsettings, &au_operator[(list[i])]);
		au_operator[(list[i])].unisonScaleFactor = _cureMidiConvCentToMul( ((i+1) >> 1) * sign * (synth_settings.unison_detune[ch]) );
		cureSynthSetNoteON(&au_operator[(list[i])], note, vel, ch);
		sign *= -1;
	}
}

void _cureMidiUnisonOn_Drum(int16_t list[UNISON_MAX_NUM], uint8_t ch, uint8_t note, uint8_t vel)
{
//	uint16_t idx_list = 0;
	static int8_t sign = 1;

	for(uint32_t i=0; i<UNISON_MAX_NUM; i++){
		if(list[i] < 0){
			break;
		}
		cureSynthOperatorCopy(&master.channel[ch].operatorsettings, &au_operator[(list[i])]);
		au_operator[(list[i])].unisonScaleFactor = _cureMidiConvCentToMul( ((i+1) >> 1) * sign * (synth_settings.unison_detune[ch]) );
		cureSynthSetNoteON_DRUM(&au_operator[(list[i])], (note << 8) + patch_drum[ note ].notenum, vel, ch);
		sign *= -1;
	}
}

void cureMidiSetNoteOn(uint8_t ch, uint8_t notenum, uint8_t vel)
{
	uint8_t i, idx_unison = 0;
//	uint8_t idx_operator;
	int16_t opnum_list[UNISON_MAX_NUM];


	// Judge the note is ON or OFF.
	//If notes are already ON, Note Off this.
	if( OPERATOR_OFF != master.channel[ch].operator_to_note.stat[notenum] ){
		cureMidiSetNoteOff(ch, notenum);
//		return;
	}

	//init unison list
	for(i=0; i<UNISON_MAX_NUM; i++){
		opnum_list[i] = -1;
	}

	//check unison number.
	if(synth_settings.unison_num[ch] > UNISON_MAX_NUM)
	{
		synth_settings.unison_num[ch] = UNISON_MAX_NUM;
	}

	//search unused operator.
	for(i=0; i<OPNUM; i++){
		if( ADSR_FINAL == au_operator[master.front_opnum].env.stat ){

			//when operator is on, note off.
			if(NOTE_ON == au_operator[master.front_opnum].stat){
				cureMidiSetNoteOff(au_operator[master.front_opnum].ch, au_operator[master.front_opnum].note_number);
			}

			//set note on status to channel structure.
			master.channel[ch].operator_to_note.stat[notenum] = master.front_opnum; //set operator number to note number structure.
			master.channel[ch].operator_to_channel.ison[master.front_opnum] = true;//set operator number to channel structure.

			opnum_list[idx_unison] = master.front_opnum;

			idx_unison++;
			if((1 + synth_settings.unison_num[ch]) <= idx_unison){
				break;
			}
		}
		master.front_opnum++;
		if(master.front_opnum >= OPNUM){
			master.front_opnum -= OPNUM;
		}

		//if synth have no vacant(==ADSR_FINAL) operators.
		if( (OPNUM - 1) == i){
			synth_voice_number_status.isFullyVoiceDetected = true;
		}
	}


	//note on
	if( TRACK_MELODY == synth_settings.tr_type[ch] ){//If sound is melody track.
		_cureMidiUnisonOn(opnum_list, ch, notenum, vel);
	}else{//If sound is drum track.

		//self off
		cureMidiSetNoteOffImmediately(ch, notenum);

		//if exclusive drum note number
		switch(notenum){
			case 42://Closed HiHat
				cureMidiSetNoteOffImmediately(ch, 44);
				cureMidiSetNoteOffImmediately(ch, 46);
				break;
			case 44://Pedaled HiHat
				cureMidiSetNoteOffImmediately(ch, 42);
				cureMidiSetNoteOffImmediately(ch, 46);
				break;
			case 46://Open Hihat
				cureMidiSetNoteOffImmediately(ch, 42);
				cureMidiSetNoteOffImmediately(ch, 44);
				break;
			case 73:
				cureMidiSetNoteOffImmediately(ch, 74);
				break;
			case 74:
				cureMidiSetNoteOffImmediately(ch, 73);
				break;
			case 80:
				cureMidiSetNoteOffImmediately(ch, 81);
				break;
			case 81:
				cureMidiSetNoteOffImmediately(ch, 80);
				break;
			default:
				break;
		}

		cureMidiSetDrumParam(ch, notenum);
		_cureMidiUnisonOn_Drum(opnum_list, ch, notenum, vel);
		//fixed to unison on
//		cureSynthSetNoteON(&au_operator[master.front_opnum], patch_drum[ val ].notenum, midi_event.data_byte[1], ch);
	}


}

void cureMidiSetNoteOff(uint8_t ch, uint8_t val)
{
	OperatorStatus *opstat;
	int8_t scale;

	opstat = &(master.channel[ch].operator_to_note.stat[val]) ;

//	if( OPERATOR_OFF != *opstat ){

		scale = val;

		for(uint16_t i=0; i<OPNUM; i++){
//old algo
//			if( (au_operator[i].ch == ch) && (au_operator[i].unisonScale == scale) ){
//				cureSynthSetNoteOFF(&au_operator[i]);
//				master.channel[ch].operator_to_channel.ison[i] = false;
//			}
			if( (master.channel[ch].operator_to_channel.ison[i]) && (au_operator[i].unisonScale == scale) ){
				cureSynthSetNoteOFF(&au_operator[i]);
				master.channel[ch].operator_to_channel.ison[i] = false;
			}
		}
		*opstat = OPERATOR_OFF;
//	}else{
//		return;
//	}
}

//use for exclusive drum note, All Sound Off.
void cureMidiSetNoteOffImmediately(uint8_t ch, uint8_t val)
{
	OperatorStatus *opstat;

	opstat = &(master.channel[ch].operator_to_note.stat[val]) ;

	for(uint16_t i=0; i<OPNUM; i++){

		if( (au_operator[i].ch == ch) && (au_operator[i].note_number == val) ){
			cureSynthSetNoteOFFImmediately(&au_operator[i]);
			master.channel[ch].operator_to_channel.ison[i] = false;
		}
	}
	*opstat = OPERATOR_OFF;

}

void cureMidiSetHold(uint8_t ch, uint8_t val)
{

	if(val >= 64){
		// hold on
		master.channel[ch].isHoldOn = true;
	}else{
		// hold off

		master.channel[ch].isHoldOn = false;//The order of clearing flag must be before cureMidiSetNoteOff()

		for(uint32_t i=0; i<128; i++){
			if( (master.channel[ch].hold_note_list[i]) ){
				cureMidiSetNoteOff(ch, i);
				master.channel[ch].hold_note_list[i] = false;
			}
		}
	}

}

void cureMidiSetSostenuto(uint8_t ch, uint8_t val)
{

	if(val >= 64){
		// Sostenuto on
		//Adding note to sostenuto_note_list
		master.channel[ch].isSostenutoOn = true;
		for(uint32_t i=0; i<128; i++){
			if(OPERATOR_OFF != master.channel[ch].operator_to_note.stat[i])
			{
				master.channel[ch].sostenuto_note_list[i] = STN_REGISTERED;
			}
		}

	}else{
		// Sostenuto off

		//The order of clearing flag must be before cureMidiSetNoteOff()
		master.channel[ch].isSostenutoOn = false;
		for(uint32_t i=0; i<128; i++){
			if( (STN_CONTINUE == master.channel[ch].sostenuto_note_list[i]) ){
					cureMidiSetNoteOff(ch, i);

			}
			master.channel[ch].sostenuto_note_list[i] = STN_INIT;
		}
	}

}

void cureMidiResetAllController(uint8_t ch)
{
	synth_settings.modulation[ch] = 0; //modulation to 0
	cureMidiSetHold(ch, 0); //hold off
	cureMidiSetSostenuto(ch, 0); //Sostenuto off
	synth_settings.expression[ch] = 0x7F; //Expression to 127
	master.channel[ch].operatorsettings.wav.pitch = (0x40 << 7); //pitch
	master.channel[ch].operatorsettings.wav.pitchScaleFactor = 1.0f; //pitch
	master.channel[ch].rpn_type = RPN_NULL; //RPN reset;
}

//channel mode message
void cureMidiAllNotesOff(uint8_t ch)
{

	//if hold is on, ignore All Notes Off
	if(master.channel[ch].isHoldOn){
		return;
	}


	for(uint32_t i=0; i<OPNUM; i++){
		if(ch == au_operator[i].ch){
			cureSynthSetNoteOFF(&au_operator[i]);
		}
	}


	//all operator on flag turn to OPERATOR_OFF in channel structure
	for(uint32_t op_idx=0; op_idx<OPNUM; op_idx++){
		master.channel[ch].operator_to_channel.ison[op_idx] = false;
	}
	//all operator status turn to OPERATOR_OFF in channel structure
	for(uint32_t val_idx=0; val_idx<128; val_idx++){
		master.channel[ch].operator_to_note.stat[val_idx] = OPERATOR_OFF;
	}


}

void cureMidiAllSoundOff(uint8_t ch)
{

	cureSynthAllSoundOff(ch);

	//all operator on flag turn to OPERATOR_OFF in channel structure
	for(uint32_t op_idx=0; op_idx<OPNUM; op_idx++){
		master.channel[ch].operator_to_channel.ison[op_idx] = false;
	}
	//all operator status turn to OPERATOR_OFF in channel structure
	for(uint32_t val_idx=0; val_idx<128; val_idx++){
		master.channel[ch].operator_to_note.stat[val_idx] = OPERATOR_OFF;
	}
}

void cureMidiSetPitchBend(uint8_t ch, uint8_t lsb, uint8_t msb)
{
	uint16_t pitchbend = ( ( msb & 0x7F) << 7) + (lsb);
	uint8_t i;

	//for pitchbend
	master.channel[ch].operatorsettings.wav.pitch = pitchbend;

	//Calculate PitchBend scalefactor
	cureSynthOperatorApplyPitchBendAndTuning( &(master.channel[ch].operatorsettings), master.channel[ch].tuning_scalefactor );

	//set pitchbend to activatied operator.
	for(i=0; i<OPNUM; i++){
//		if( true == master.channel[ch].operator_to_channel.ison[i] ){
//			au_operator[i].wav.pitch = pitchbend;
//			//added tuning scale factor from master fine/coarse tuning.
//			au_operator[i].wav.pitchScaleFactor = master.channel[ch].operatorsettings.wav.pitchScaleFactor;
//		}

		//algo changed.
		if( ch == au_operator[i].ch ){
			au_operator[i].wav.pitch = pitchbend;
			//added tuning scale factor from master fine/coarse tuning.
			au_operator[i].wav.pitchScaleFactor = master.channel[ch].operatorsettings.wav.pitchScaleFactor;
		}
	}

}

void cureMidiSetTrianglePeak(uint8_t ch, uint8_t val)
{
	uint16_t ptime = val;
	uint8_t i;

	//set peak time(ptime) to operator setting
	master.channel[ch].operatorsettings.wav.triangle_peak_time = ptime;
	master.channel[ch].operatorsettings.wav.triangle_peak_time_buf = ptime;

	//set peak time to activated operator.
	for(i=0; i<OPNUM; i++){
		if( true == master.channel[ch].operator_to_channel.ison[i] ){
			au_operator[i].wav.triangle_peak_time_buf = ptime;
		}
	}

}

void cureMidiSetRingmodMultiply(uint8_t ch, uint8_t val)
{
	float buf_float = cureMidiRingmodMultofloat(val);
	GM1_EXCEPT(master.channel[ch].operatorsettings.ringmod.multiply = buf_float);
	for(uint32_t i=0; i<OPNUM; i++){
		if(true == master.channel[ch].operator_to_channel.ison[i]){
			GM1_EXCEPT(au_operator[i].ringmod.multiply = buf_float );
		}
	}
}

void cureMidiSetRingmodGain(uint8_t ch, uint8_t val)
{
	uint8_t buf_uint8 = (val & 0x7F);
		GM1_EXCEPT(master.channel[ch].operatorsettings.ringmod.gain = buf_uint8);

		for(uint32_t i=0; i<OPNUM; i++){
			if(true == master.channel[ch].operator_to_channel.ison[i]){
				GM1_EXCEPT(au_operator[i].ringmod.gain = buf_uint8);
			}
		}
}

void cureMidiSetWaveType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			master.channel[ch].operatorsettings.wav.type = SINE;
			break;
		case 1:
			master.channel[ch].operatorsettings.wav.type = SQUARE;
			break;
		case 2:
			master.channel[ch].operatorsettings.wav.type = TRIANGLE;
			break;
		case 3:
			master.channel[ch].operatorsettings.wav.type = SAW;
			break;
		case 4:
			master.channel[ch].operatorsettings.wav.type = WHITE_NOISE;
			break;
		case 5:
			master.channel[ch].operatorsettings.wav.type = PINK_NOISE;
			break;
		case 6:
			master.channel[ch].operatorsettings.wav.type = PCM;
			break;
		case 7:
			master.channel[ch].operatorsettings.wav.type = PCM_REV;
			break;
		default:
			break;
	}
}

void cureMidiSetWaveNum(uint8_t ch, uint8_t num)
{
	if( PCM_NUM > num){
		master.channel[ch].operatorsettings.wavenum = num;
	}
}

void cureMidiSetSweepType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_UP;
			break;
		case 1:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_DOWN;
			break;
		case 2:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_NONE;
			break;
		default:
			break;
	}
}

void cureMidiSetDistortionType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			synth_settings.dst_mode[ch] = DIST_OFF;
			break;
		case 1:
			synth_settings.dst_mode[ch] = DIST_HARD;
			break;
		case 2:
			synth_settings.dst_mode[ch] = DIST_SOFT;
			break;
		case 3:
			synth_settings.dst_mode[ch] = DIST_ASSYM;
			break;
		default:
			synth_settings.dst_mode[ch] = DIST_OFF;
			break;
			break;
	}
}

void cureSynthSetDrumTrack(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			synth_settings.tr_type[ch] = TRACK_MELODY;
			cureMidiSetProgramChange(ch, 0x00);
			break;
		case 1:
			synth_settings.tr_type[ch] = TRACK_DRUM;
			break;
		default:
			break;
	}

}


void cureMidiRPNSelect(uint8_t ch)
{

	master.channel[ch].rpn_nrpn_choose = CHOSE_RPN;

	if(0x0 == master.channel[ch].rpn_msb){
		if(0x0 == master.channel[ch].rpn_lsb){
			master.channel[ch].rpn_type = RPN_PITCH_SENS;
		}else if(0x1 == master.channel[ch].rpn_lsb){
			master.channel[ch].rpn_type = RPN_FINE_TUNE;
		}else if(0x2 == master.channel[ch].rpn_lsb){
			master.channel[ch].rpn_type = RPN_COARSE_TUNE;
		}

	}else if( (0x7F == master.channel[ch].rpn_msb) && (0x7F == master.channel[ch].rpn_lsb) ){
		master.channel[ch].rpn_type = RPN_NULL;
		master.channel[ch].nrpn_type = NRPN_NULL;// RPN null (MSB/LSB:127/127) has also NRPN null.
	}else{
		master.channel[ch].rpn_type = RPN_UNDEFINED;//undefined rpn number received.
	}

}

void cureMidiNRPNSelect(uint8_t ch)
{
	master.channel[ch].rpn_nrpn_choose = CHOSE_NRPN;

	uint16_t nrpn_num = ((master.channel[ch].nrpn_msb & 0xFF) << 8) + master.channel[ch].nrpn_lsb;

	switch(nrpn_num){
		case 0x0200:
			master.channel[ch].nrpn_type = NRPN_WAVE_TYPE;
			break;
		case 0x0201:
			master.channel[ch].nrpn_type = NRPN_WAVE_NUM;
			break;
		case 0x0202:
			master.channel[ch].nrpn_type = NRPN_OUTPUT_GAIN;
			break;
		case 0x0300:
			master.channel[ch].nrpn_type = NRPN_RING_MULTIPLY;
			break;
		case 0x0301:
			master.channel[ch].nrpn_type = NRPN_RING_GAIN;
			break;
		case 0x0400:
			master.channel[ch].nrpn_type = NRPN_SWEEP_TYPE;
			break;
		case 0x0401:
			master.channel[ch].nrpn_type = NRPN_SWEEP_SPD;
			break;
		case 0x0500:
			master.channel[ch].nrpn_type = NRPN_TRI_PEAKTIME;
			break;
		case 0x0501:
			master.channel[ch].nrpn_type = NRPN_SQU_DUTY;
			break;
		case 0x0600:
			master.channel[ch].nrpn_type = NRPN_SUSTAIN_RATE;
			break;
		case 0x0601:
			master.channel[ch].nrpn_type = NRPN_SUSTAIN_LEVEL;
			break;
		case 0x0700:
			master.channel[ch].nrpn_type = NRPN_DELAY_TIME;
			break;
		case 0x0701:
			master.channel[ch].nrpn_type = NRPN_FEEDBK_GAIN;
			break;
		case 0x070A:
			master.channel[ch].nrpn_type = NRPN_CHORUS_DEPTH;
			break;
		case 0x070B:
			master.channel[ch].nrpn_type = NRPN_CHORUS_FREQ;
			break;
		case 0x0714:
			master.channel[ch].nrpn_type = NRPN_REVERB_TYPE;
			break;
		case 0x0800:
			master.channel[ch].nrpn_type = NRPN_DIST_TYPE;
			break;
		case 0x0801:
			master.channel[ch].nrpn_type = NRPN_DIST_LEVEL;
			break;
		case 0x0802:
			master.channel[ch].nrpn_type = NRPN_DIST_GAIN;
			break;
		case 0x0803:
			master.channel[ch].nrpn_type = NRPN_DIST_CHANNEL;
			break;
		case 0x0900:
			master.channel[ch].nrpn_type = NRPN_UNISON_NUM;
			break;
		case 0x0901:
			master.channel[ch].nrpn_type = NRPN_UNISON_DET;
			break;
		case 0x0A00:
			master.channel[ch].nrpn_type = NRPN_DRUM_TRACK;
			break;
		default:
			master.channel[ch].nrpn_type = NRPN_UNDEFINED;
			break;
	}

}

void cureMidiCalcTuning(uint8_t ch)
{
	float cent=0.0f;

	int16_t fine_tune_value = ((int16_t)master.channel[ch].fine_tune_msb - 64) * 128 + master.channel[ch].fine_tune_lsb;
	cent += (float)fine_tune_value * 100.0f / 8192.0f;

	int16_t coarse_tune_value = (int16_t)master.channel[ch].coarse_tune_msb - 64;
	cent += (float)coarse_tune_value * 100.0f;

	master.channel[ch].tuning_scalefactor = powf(2.0f, cent / 1200.0f);
	cureSynthOperatorApplyPitchBendAndTuning( &(master.channel[ch].operatorsettings), master.channel[ch].tuning_scalefactor );

}

void cureMidiSetDataEntryMSB(uint8_t ch, uint8_t val)
{
	//RPN
	if(CHOSE_RPN == master.channel[ch].rpn_nrpn_choose){
		switch(master.channel[ch].rpn_type){
			case RPN_PITCH_SENS:
				master.channel[ch].operatorsettings.wav.pitchbendsensitivity = val & 0x7F;
				break;
			case RPN_FINE_TUNE:
				master.channel[ch].fine_tune_msb = val & 0x7F;
				cureMidiCalcTuning(ch);
				break;
			case RPN_COARSE_TUNE:
				master.channel[ch].coarse_tune_msb = val & 0x7F;
				cureMidiCalcTuning(ch);
				break;
			default:
				break;
		}
	//NRPN
	}else{
		switch(master.channel[ch].nrpn_type){
			case NRPN_WAVE_TYPE:
				cureMidiSetWaveType( ch, (val & 0x7F) );
				break;
			case NRPN_WAVE_NUM:
				cureMidiSetWaveNum( ch, (val & 0x7F) );
				break;
			case NRPN_OUTPUT_GAIN:
				master.channel[ch].operatorsettings.out_gain = (val & 0x7F);
				break;
			case NRPN_RING_MULTIPLY:
				cureMidiSetRingmodMultiply(ch, val);
				break;
			case NRPN_RING_GAIN:
				cureMidiSetRingmodGain(ch, val);
				break;
			case NRPN_SWEEP_TYPE:
				GM1_EXCEPT(cureMidiSetSweepType( ch, (val & 0x7F) ));
				break;
			case NRPN_SWEEP_SPD:
				GM1_EXCEPT(master.channel[ch].operatorsettings.wav.pitch_sweep_spd  = (val & 0x7F));
				break;
			case NRPN_TRI_PEAKTIME:
				cureMidiSetTrianglePeak(ch, val);
				break;
			case NRPN_SQU_DUTY:
				break;
			case NRPN_SUSTAIN_RATE:
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.sustainRate = (val & 0x7F) * 2);
				break;
			case NRPN_SUSTAIN_LEVEL:
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.sustainLevel = (val & 0x7F) * 2);
				break;
			case NRPN_DELAY_TIME:
				efxSetDelayTime(val & 0x7F);
				break;
			case NRPN_FEEDBK_GAIN:
				efxSetDelayFdbkGain((val & 0x7F));
				break;
			case NRPN_DIST_TYPE:
				cureMidiSetDistortionType( ch, (val & 0x7F) );
				break;
			case NRPN_DIST_LEVEL:
				synth_settings.dst_level[ch] = (val & 0x7F);
				break;
			case NRPN_CHORUS_DEPTH:
				efxSetChorusDepth(val & 0x7F);
				break;
			case NRPN_CHORUS_FREQ:
				efxSetChorusFreq(val & 0x7F);
				break;
			case NRPN_REVERB_TYPE:
				efxSetReverbPreset(val & 0x7F);
				break;
			case NRPN_DIST_GAIN:
				synth_settings.dst_gain[ch] = (val & 0x7F);
				break;
			case NRPN_DIST_CHANNEL:
				break;
			case NRPN_UNISON_NUM:
				synth_settings.unison_num[ch] = (val & 0x7F);
				break;
			case NRPN_UNISON_DET:
				synth_settings.unison_detune[ch] = (val & 0x7F) - 64;
				break;
			case NRPN_DRUM_TRACK:
				cureSynthSetDrumTrack( ch, (val & 0x7F) );
				break;
			default:
				break;
		}
	}

}

void cureMidiSetDataEntryLSB(uint8_t ch, uint8_t val)
{
	//RPN
	if(CHOSE_RPN == master.channel[ch].rpn_nrpn_choose){
		switch(master.channel[ch].rpn_type){
			case RPN_PITCH_SENS:
				//ignore
				break;
			case RPN_FINE_TUNE:
				master.channel[ch].fine_tune_lsb = val & 0x7F;
				cureMidiCalcTuning(ch);
				break;
			case RPN_COARSE_TUNE:
				//ignore
				break;
			default:
				break;
		}
	//NRPN
	}else{
		switch(master.channel[ch].nrpn_type){
			case NRPN_WAVE_TYPE:
				break;
			case NRPN_OUTPUT_GAIN:
				break;
			case NRPN_RING_MULTIPLY:
				break;
			case NRPN_RING_GAIN:
				break;
			case NRPN_SWEEP_TYPE:
				break;
			case NRPN_SWEEP_SPD:
				break;
			case NRPN_TRI_PEAKTIME:
				break;
			case NRPN_SQU_DUTY:
				break;
			case NRPN_SUSTAIN_RATE:
				break;
			case NRPN_SUSTAIN_LEVEL:
				break;
			case NRPN_DELAY_TIME:
				break;
			case NRPN_FEEDBK_GAIN:
				break;
			case NRPN_CHORUS_DEPTH:
				break;
			case NRPN_CHORUS_FREQ:
				break;
			case NRPN_REVERB_TYPE:
				break;
			case NRPN_DIST_TYPE:
				break;
			case NRPN_DIST_LEVEL:
				break;
			case NRPN_DIST_GAIN:
				break;
			case NRPN_DIST_CHANNEL:
				break;
			case NRPN_UNISON_NUM:
				break;
			case NRPN_UNISON_DET:
				break;
			case NRPN_DRUM_TRACK:
				break;
			default:
				break;
		}
	}

}

void cureMidiSetDataIncrement(uint8_t ch)
{
	if(CHOSE_RPN == master.channel[ch].rpn_nrpn_choose){
		switch(master.channel[ch].rpn_type){
			case RPN_PITCH_SENS:
				break;
			case RPN_FINE_TUNE:
				break;
			case RPN_COARSE_TUNE:
				break;
			default:
				break;
		}
	}else{

	}

}

void cureMidiSetDataDecrement(uint8_t ch)
{
	if(CHOSE_RPN == master.channel[ch].rpn_nrpn_choose){
		switch(master.channel[ch].rpn_type){
			case RPN_PITCH_SENS:
				break;
			case RPN_FINE_TUNE:
				break;
			case RPN_COARSE_TUNE:
				break;
			default:
				break;
		}
	}else{

	}

}

void cureMidiSetControlChange(uint8_t ch, uint8_t ccno, uint8_t val)
{
		switch(ccno){
			case 1://Modulation
				synth_settings.modulation[ch] = (val & 0x7F);
				break;
			case 7://Volume
				synth_settings.volume_ref[ch] = (val & 0x7F);
				break;
			case 10://Pan
				synth_settings.pan_ref[ch] = (val & 0x7F);
				break;
			case 11://Expression
				synth_settings.expression_ref[ch] = (val & 0x7F);
				break;
			case 20://Cure Wave Type
				cureMidiSetWaveType( ch, (val & 0x7F) );
				break;
			case 23://Cure Output Gain
				master.channel[ch].operatorsettings.out_gain = (val & 0x7F);
				break;
			case 25 ://Cure Distortion Type
				cureMidiSetDistortionType( ch, (val & 0x7F) );
				break;
			case 26 ://Cure Distortion Level
				synth_settings.dst_level[ch] = (val & 0x7F);
				break;
			case 27 ://Cure Distortion Gain
				synth_settings.dst_gain[ch] = (val & 0x7F);
				break;
			case 52 ://Cure Unison Number
				synth_settings.unison_num[ch] = (val & 0x7F);
				break;
			case 53 ://Cure Unison detune
				synth_settings.unison_detune[ch] = (val & 0x7F) - 64;
				break;
			case 64 ://Hold
				cureMidiSetHold(ch, val & 0x7F);
				break;
			case 66 ://Sostenuto
				cureMidiSetSostenuto(ch, val & 0x7F);;
				break;
			case 71 ://Resonance
				synth_settings.resonance[ch] = (val & 0x7F);
				cureSynthSetFilter(ch);
				break;
			case 72 ://Release time
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.release = (val & 0x7F) * 2);
				break;
			case 73 ://Attack time
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.attack = (val & 0x7F) * 2);
				break;
			case 74 ://Cut off
				synth_settings.cutoff[ch] = (val & 0x7F);
				cureSynthSetFilter(ch);
				break;
			case 75 ://Decay time
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.decay = (val & 0x7F) * 2);
				break;
			case 76 ://Modulation Rate
				synth_settings.modulation_rate[ch] = (val & 0x7F);
				break;
			case 85 ://Cure Triangle Peak(original)
				cureMidiSetTrianglePeak(ch, val);
				break;
			case 91 ://Reverb
				synth_settings.reverb_level[ch] = (val & 0x7F);
				break;
			case 93 ://Chorus
				synth_settings.chorus_level[ch] = (val & 0x7F);
				break;
			case 94 ://Delay
				synth_settings.delay_level[ch] = (val & 0x7F);
				break;
			case 102 ://SustainRate(original)
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.sustainRate = (val & 0x7F) * 2);
				break;
			case 103 ://SustainLevel(original)
				GM1_EXCEPT(master.channel[ch].operatorsettings.env.sustainLevel = (val & 0x7F) * 2);
				break;
			case 104 ://RingModulator Multiply(original)
				cureMidiSetRingmodMultiply(ch, val);
				break;
			case 105 ://RingModulator Gain(original)
				cureMidiSetRingmodGain(ch, val);
				break;
			case 108 ://Sweep type(original)
				GM1_EXCEPT(cureMidiSetSweepType( ch, (val & 0x7F) ));
				break;
			case 109 ://Sweep speed(original)
				GM1_EXCEPT(master.channel[ch].operatorsettings.wav.pitch_sweep_spd  = (val & 0x7F));
				break;
			case 117 ://Drum Track Setting(original)
				cureSynthSetDrumTrack(ch, (val & 0x7F));
				break;
			case 120 ://All Sound Off
				cureMidiAllSoundOff(ch);
				break;
			case 121 ://Reset All Controller
//				synth_settings.modulation[ch] = 0; //modulation to 0
//				cureMidiSetHold(ch, 0); //hold off
//				synth_settings.expression[ch] = 0x7F; //Expression to 127
//				master.channel[ch].operatorsettings.wav.pitch = (0x40 << 7); //pitch
//				master.channel[ch].operatorsettings.wav.pitchScaleFactor = 1.0f; //pitch
//				master.channel[ch].rpn_type = RPN_NULL; //RPN reset;
				cureMidiResetAllController(ch);
				break;
			case 123 ://All Notes Off
			case 124 ://OMNI OFF
			case 125 ://OMNI ON
			case 126 ://MONO
			case 127 ://POLY
				cureMidiAllNotesOff(ch);
				break;
			//RPN messages
			case 98:// NRPN LSB
				master.channel[ch].nrpn_lsb = (val & 0x7F);
				cureMidiNRPNSelect(ch);
				break;
			case 99: // NRPN MSB
				master.channel[ch].nrpn_msb = (val & 0x7F);
				cureMidiNRPNSelect(ch);
				break;
			//RPN messages
			case 100:// RPN LSB
				master.channel[ch].rpn_lsb = (val & 0x7F);
				cureMidiRPNSelect(ch);
				break;
			case 101: // RPN MSB
				master.channel[ch].rpn_msb = (val & 0x7F);
				cureMidiRPNSelect(ch);
				break;
			case 6: // Data entry MSB
				cureMidiSetDataEntryMSB(ch, val);
				break;
			case 38:// Data entry LSB
				cureMidiSetDataEntryLSB(ch, val);
				break;
			case 96:// Data increment todo
				cureMidiSetDataIncrement(ch);
				break;
			case 97:// Data decrement todo
				cureMidiSetDataDecrement(ch);
				break;
			default:
				break;
		}



}

void cureMidiSetProgramChange(uint8_t ch, uint8_t pgno)
{
	uint8_t i;

	if( MODE_ORIGINAL == synth_settings.mode ){
		for(i=0; i<OPNUM; i++){
			if( false == master.channel[ch].operator_to_channel.ison[i] ){ // if operator is not processing.

				switch(pgno){
					case 0://SINE
						master.channel[ch].operatorsettings.wav.type = SINE;
						break;
					case 1://Volume
						master.channel[ch].operatorsettings.wav.type = SQUARE;
						break;
					case 2://Volume
						master.channel[ch].operatorsettings.wav.type = TRIANGLE;
						break;
					case 3://Volume
						master.channel[ch].operatorsettings.wav.type = SAW;
						break;
					case 4://Volume
						master.channel[ch].operatorsettings.wav.type = WHITE_NOISE;
						break;
					case 5://Volume
						master.channel[ch].operatorsettings.wav.type = PINK_NOISE;
						break;
				}

			}
		}
	}else if( (MODE_GM1 == synth_settings.mode) || (MODE_HYBRID == synth_settings.mode) ){

			master.channel[ch].operatorsettings.wav.type 				= patch_melody[ pgno ].wtype;
			master.channel[ch].operatorsettings.ntype 					= patch_melody[ pgno ].ntype;
			master.channel[ch].operatorsettings.wavenum				    = patch_melody[ pgno ].wavenum;
			master.channel[ch].operatorsettings.ringmod.multiply 		= cureMidiRingmodMultofloat(patch_melody[ pgno ].ringmod_multiply);
			master.channel[ch].operatorsettings.ringmod.gain 			= patch_melody[ pgno ].ringmod_gain;
			master.channel[ch].operatorsettings.wav.pitch_sweep_type 	= patch_melody[ pgno ].pitch_sweep_type;
			master.channel[ch].operatorsettings.wav.pitch_sweep_spd 	= patch_melody[ pgno ].pitch_sweep_spd;
			master.channel[ch].operatorsettings.env.attack 				= patch_melody[ pgno ].attack * 2;
			master.channel[ch].operatorsettings.env.decay 				= patch_melody[ pgno ].decay * 2;
			master.channel[ch].operatorsettings.env.sustainLevel 		= patch_melody[ pgno ].sustainLevel * 2;
			master.channel[ch].operatorsettings.env.sustainRate 		= patch_melody[ pgno ].sustainRate * 2;
			master.channel[ch].operatorsettings.env.release 			= patch_melody[ pgno ].release * 2;
			master.channel[ch].operatorsettings.out_gain	 			= patch_melody[ pgno ].out_gain;
	}

}

//FUNC_STATUS cureMidiCompareSysEx(const uint8_t * dat, uint8_t len)
//{
//	uint32_t i;
//
//	if(len != analyzed_status.data_idx){
//		return FUNC_ERROR;
//	}
//
//	for(i=0; i<len; i++){
//		if( dat[i] != midi_event.data_byte[i]){
//			return FUNC_ERROR;
//		}
//	}
//
//	return FUNC_SUCCESS;
//
//}

//
//len: compare length
//start_byte_position: start position of data byte
FUNC_STATUS cureMidiCompareSysEx(MIDIEvent* event, const uint8_t * dat, uint8_t len, uint8_t start_databyte_pos)
{
	uint32_t i;

	for(i=0; i<len; i++){
		if( dat[i] != event->data_byte[(start_databyte_pos-1) + i]){
			return FUNC_ERROR;
		}
	}

	return FUNC_SUCCESS;

}


void cureMidiResetSynth()
{
	cureMidiMasterChannelInit(&master);
	dpChangeInfoInit(&midi_ui_setting_info);
	for(uint32_t i=0; i<OPNUM; i++){
		cureSynthOperatorInit(&au_operator[i]);
	}
	cureSynthSettingInit();

	//Program change init
	for(uint32_t i=0; i<CHNUM; i++){
		cureMidiSetProgramChange(i, 0x00);
	}

	efxSetParameterInit();

}

//if use this method, don't use cureMidiBufferEnqueue/Dequeue, cureMidiBufferIsEmpty
//void cureMidiDirectInput(uint8_t midibyte)
//{
//	midi_buf = midibyte;
//}

void cureMidiResetFromBufferFull()
{
	for(uint32_t i=0; i<CHNUM; i++){
		cureMidiAllNotesOff(i);
//		cureMidiResetAllController(i);
	}
	for(uint32_t i=0; i<MIDI_INPUT_SRC_NUM; i++){
		midi_event[i].type = analyzed_status[i].type = MSG_NOTHING;
		analyzed_status[i].stat = START_ANALYSIS;
	}
	synth_settings.disp_msg_type = DSP_BUFFER_FULL;

}

void cureMidiResetFromOverload()
{
	cureMidiResetSynth();

	for(uint32_t i=0; i<MIDI_INPUT_SRC_NUM; i++){
		midi_event[i].type = analyzed_status[i].type = MSG_NOTHING;
		analyzed_status[i].stat = START_ANALYSIS;
	}
	synth_settings.disp_msg_type = DSP_OVERLOAD;
}

//TODO:Now under construction.
void cureMidiSetSystemExclusive(MidiAnalysisStatus* stat, MIDIEvent* event)
{
//	const uint8_t gm_system_on[] = {0x7E, 0x7F, 0x09, 0x01};
//	const uint8_t gm2_system_on[] = {0x7E, 0x7F, 0x09, 0x03};
//	const uint8_t gs_reset[] = {0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41};
//	const uint8_t xg_system_on[] = {0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41};
//	const uint8_t original_mode[] = {0x7D, 0x01, 0x02, 0x03, 0x04};
//	const uint8_t hybrid_mode[] = {0x7D, 0x01, 0x02, 0x03, 0x05};
//	const uint8_t drum_track_melody[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x00,0x11};
//	const uint8_t drum_track_on_1[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x01,0x10};
//	const uint8_t drum_track_on_2[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x02,0x0F};

	const uint8_t gm_system_on[] = {0x7F, 0x09, 0x01};//start second byte of SysEx
	const uint8_t gm2_system_on[] = {0x7F, 0x09, 0x03};//start second byte of SysEx
	const uint8_t gs_reset[] = {0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41};//start third byte of SysEx
	const uint8_t system_mode1[] = {0x42, 0x12, 0x00, 0x00, 0x7F, 0x00, 0x01};//start third byte of SysEx
	const uint8_t system_mode2[] = {0x42, 0x12, 0x00, 0x00, 0x7F, 0x01, 0x00};//start third byte of SysEx
	const uint8_t xg_system_on[] = {0x4C, 0x00, 0x00, 0x7E, 0x00};//start third byte of SysEx
	const uint8_t original_mode[] = {0x01, 0x02, 0x03, 0x04};//start second byte of SysEx, original, experimental
	const uint8_t hybrid_mode[] = {0x01, 0x02, 0x03, 0x05};//start second byte of SysEx, original, experimental
	const uint8_t curesynth_ID[] = {0x00, 0x48, 0x05};//todo, AMEI, start second byte of SysEx
	const uint8_t master_volume[] = {0x7F, 0x04, 0x01};//start second byte of SysEx


	uint16_t jp_character_halfbyte_length;



	//Curesynth manufacture ID's exclusive judge
	bool is_curesynth_id_exclusive = true;

	if( 4 >= stat->data_idx){//data must be more than 4byte(00h, 48h, 05h, dev_id)
		is_curesynth_id_exclusive = false;
	}

	for(uint32_t i=0; i<3; i++)
	{
		if(event->data_byte[i] != curesynth_ID[i]){
			is_curesynth_id_exclusive = false;
			break;
		}
	}


	//Curesynth manufacture ID's exclusive
	if(is_curesynth_id_exclusive)
	{
		switch(event->data_byte[4]){

			//Mode Set
			case 0x00:
				if(6 != stat->data_idx){
					break;
				}

				if(0x00 == event->data_byte[5]){//CureSynth Hybrid Mode

					//Initialize
					cureMidiResetSynth();

					//set hybrid system on
					synth_settings.mode = MODE_HYBRID;
					synth_settings.disp_msg_type = DSP_CURE_HYBRID;
					midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_DEFAULT;
					break;
				}

				break;

			//UI Type Set
			case 0x03:
				if(6 != stat->data_idx){
					break;
				}
				if(0x00 == event->data_byte[5]){
					midi_ui_setting_info.ui_change_state = UI_CHANGE_TO_TYPE_A;
					break;
				}else if(0x01 == event->data_byte[5] ){
					midi_ui_setting_info.ui_change_state = UI_CHANGE_TO_TYPE_B;
					break;
				}else if(0x02 == event->data_byte[5] ){
					midi_ui_setting_info.ui_change_state = UI_CHANGE_TO_PIANOROLL;
					break;
				}else if(0x03 == event->data_byte[5] ){
					midi_ui_setting_info.ui_change_state = UI_CHANGE_TO_WAVE;
					break;
				}
				break;

			//Face Blinking Set
			case 0x04:
				if(6 != stat->data_idx){
					break;
				}
				if(0x00 == event->data_byte[5]){
					midi_ui_setting_info.eye_blink_state = EYE_BLINK_TURN_OFF;
					break;
				}else if(0x01 == event->data_byte[5] ){
					midi_ui_setting_info.eye_blink_state = EYE_BLINK_TURN_ON;
					break;
				}
				break;

			//Japanese character mode
			case 0x05:
				if(6 != stat->data_idx){
					break;
				}
				if(0x00 == event->data_byte[5]){
					midi_ui_setting_info.language_change_state = LANG_CHANGE_TO_EN;
					break;
				}else if(0x01 == event->data_byte[5] ){
					midi_ui_setting_info.language_change_state = LANG_CHANGE_TO_JP;
					string_ref_upper[0] = midi_ui_setting_info.string_upper[0] = '\0';
					string_ref_downer[0] = midi_ui_setting_info.string_lower[0] =  '\0';
					string_upper_ptr = 0;
					string_downer_ptr = 0;
					break;
				}
				break;

			//Japanese character Delete TODO
			case 0x06:
				if(6 != stat->data_idx){
					break;
				}
				switch(event->data_byte[5]){
					case 0x00://delete all
						string_ref_upper[0] = midi_ui_setting_info.string_upper[0] = '\0';
						string_ref_downer[0] = midi_ui_setting_info.string_lower[0] =  '\0';
						midi_ui_setting_info.language_control_state_upper = LANG_CTRL_UPDATE;
						midi_ui_setting_info.language_control_state_lower = LANG_CTRL_UPDATE;
						string_upper_ptr = 0;
						string_downer_ptr = 0;
						break;
					case 0x01://delete upper
						string_ref_upper[0] = midi_ui_setting_info.string_upper[0] = '\0';
						midi_ui_setting_info.language_control_state_upper = LANG_CTRL_UPDATE;
						string_upper_ptr = 0;
						break;
						break;
					case 0x02://delete lower
						string_ref_downer[0] = midi_ui_setting_info.string_lower[0] =  '\0';
						midi_ui_setting_info.language_control_state_lower = LANG_CTRL_UPDATE;
						string_downer_ptr = 0;
						break;
						break;

				}

				break;

			//Japanese character Add TODO
			case 0x07:
				if(7 > stat->data_idx){
					break;
				}

				uint8_t char_counter = 0;
				uint8_t databyte_counter = 6;

				switch(event->data_byte[5]){
					case 0x00://add upper
						jp_character_halfbyte_length = stat->data_idx - 6;

						while(char_counter < (jp_character_halfbyte_length >> 1)){
							string_ref_upper[string_upper_ptr]
								= ((event->data_byte[databyte_counter] & 0x0F) << 4) + (event->data_byte[databyte_counter+1] & 0x0F);
							string_ref_upper[string_upper_ptr+1] = '\0';
							char_counter++;
							string_upper_ptr++;
							databyte_counter += 2;

							if(DISPSETTING_STRING_MAX_WIDTH < string_upper_ptr ){
								string_upper_ptr = DISPSETTING_STRING_MAX_WIDTH;
								string_ref_upper[string_upper_ptr] = '\0';
								break;
							}
						}
						midi_ui_setting_info.language_control_state_upper = LANG_CTRL_UPDATE;
						strcpy(midi_ui_setting_info.string_upper, string_ref_upper);
						break;
					case 0x01://add lower
						jp_character_halfbyte_length = stat->data_idx - 6;

						while(char_counter < (jp_character_halfbyte_length >> 1)){
							string_ref_downer[string_downer_ptr]
								= ((event->data_byte[databyte_counter] & 0x0F) << 4) + (event->data_byte[databyte_counter+1] & 0x0F);
							string_ref_downer[string_downer_ptr+1] = '\0';
							char_counter++;
							string_downer_ptr++;
							databyte_counter += 2;

							if(DISPSETTING_STRING_MAX_WIDTH < string_downer_ptr ){
								string_downer_ptr = DISPSETTING_STRING_MAX_WIDTH;
								string_ref_downer[string_downer_ptr] = '\0';
								break;
							}
						}
						midi_ui_setting_info.language_control_state_lower = LANG_CTRL_UPDATE;
						strcpy(midi_ui_setting_info.string_lower, string_ref_downer);
						break;
					default:
						break;
				}

				break;
			case 0x10:
				if(7 > stat->data_idx){
					break;
				}

				switch(event->data_byte[5]){
					case 0x00://set to default
						midi_ui_setting_info.eye_change_num = 0;
						midi_ui_setting_info.eyebrow_change_num = 0;
						midi_ui_setting_info.mouth_change_num = 0;
						break;

					case 0x01://eyebrow
						midi_ui_setting_info.eyebrow_change_num = event->data_byte[6];
						break;

					case 0x02://eye
						midi_ui_setting_info.eye_change_num = event->data_byte[6];
						break;

					case 0x03://mouth
						midi_ui_setting_info.mouth_change_num = event->data_byte[6];
						break;

					default:
						break;
				}

				break;

			//Debug Info Displaying
			case 0x70:
				if(6 > stat->data_idx){
						break;
					}

				switch(event->data_byte[5]){
					case 0x00://set to default
						is_debug_displaying = false;
						break;

					case 0x01://eyebrow
						is_debug_displaying = true;
						break;

					default:
						break;
				}
				break;

			default:
				break;
		}

		return;
	}

	//Other system exclusive
	switch(event->data_byte[0]){

		//realtime universal system exclusive
		case 0x7F:
			//Master Volume
			if( (FUNC_SUCCESS == cureMidiCompareSysEx(event, master_volume, sizeof(master_volume), 2))	)
			{
				if(6 != stat->data_idx){
					break;
				}
				//Change Master Volume
				synth_settings.master_volume = (event->data_byte[5] & 0xFF);
				return;
			}
			break;
		//non-realtime universal system exclusive
		case 0x7E:
			if( (FUNC_SUCCESS == cureMidiCompareSysEx(event, gm_system_on, sizeof(gm_system_on), 2))
					|| (FUNC_SUCCESS == cureMidiCompareSysEx(event, gm2_system_on, sizeof(gm2_system_on), 2))
					){

				//Initialize
				cureMidiResetSynth();

				//set gm system on
				synth_settings.mode = MODE_GM1;
				synth_settings.disp_msg_type = DSP_GM;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_DEFAULT;
				return;
			}

			break;

		//Roland
		case 0x41:
			if( FUNC_SUCCESS == cureMidiCompareSysEx(event, gs_reset, sizeof(gs_reset), 3) ){

				//Initialize
				cureMidiResetSynth();

				//set gm system on
				synth_settings.mode = MODE_GM1;
				synth_settings.disp_msg_type = DSP_GS;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_GS;
				return;
			}else if( FUNC_SUCCESS == cureMidiCompareSysEx(event, system_mode1, sizeof(system_mode1), 3) ){

				//Initialize
				cureMidiResetSynth();

				//set gm system on
				synth_settings.mode = MODE_GM1;
				synth_settings.disp_msg_type = DSP_SYSMODE1;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_GS;
				return;
			}else if( FUNC_SUCCESS == cureMidiCompareSysEx(event, system_mode2, sizeof(system_mode2), 3) ){

				//Initialize
				cureMidiResetSynth();

				//set gm system on
				synth_settings.mode = MODE_GM1;
				synth_settings.disp_msg_type = DSP_SYSMODE2;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_GS;
				return;
			}
			break;

		//YAMAHA
		case 0x43:
			if(0x10 != (event->data_byte[1] & 0xF0) ){
				return;
			}
			if( FUNC_SUCCESS == cureMidiCompareSysEx(event, xg_system_on, sizeof(xg_system_on), 3) ){

				//Initialize
				cureMidiResetSynth();

				//set gm system on
				synth_settings.mode = MODE_GM1;
				synth_settings.disp_msg_type = DSP_XG;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_XG;
				return;
			}
			break;

		//CureSynth(experimental)
		case 0x7D:
			if( FUNC_SUCCESS == cureMidiCompareSysEx(event, original_mode, sizeof(original_mode), 2) ){

				//Initialize
				cureMidiResetSynth();

				//set original system on
				synth_settings.mode = MODE_ORIGINAL;
				synth_settings.disp_msg_type = DSP_CURE_ORIG;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_DEFAULT;
				return;

			}else if( FUNC_SUCCESS == cureMidiCompareSysEx(event, hybrid_mode, sizeof(hybrid_mode), 2) ){

				//Initialize
				cureMidiResetSynth();

				//set hybrid system on
				synth_settings.mode = MODE_HYBRID;
				synth_settings.disp_msg_type = DSP_CURE_HYBRID;
				midi_ui_setting_info.display_color_change_state = COL_CHANGE_TO_DEFAULT;
				return;
			}
			break;

		default:
			break;
	}


#ifdef OLD_PROCESS
	if( (FUNC_SUCCESS == cureMidiCompareSysEx(gm_system_on, sizeof(gm_system_on)))
			|| (FUNC_SUCCESS == cureMidiCompareSysEx(gm2_system_on, sizeof(gm2_system_on)))
			|| (FUNC_SUCCESS == cureMidiCompareSysEx(gs_reset, sizeof(gs_reset)))
			){

		//Initialize
		cureMidiResetSynth();

		//set gm system on
		synth_settings.mode = MODE_GM1;
		return;

	}else if(FUNC_SUCCESS == cureMidiCompareSysEx(original_mode, sizeof(original_mode))){

		//Initialize
		cureMidiResetSynth();

		//set original system on
		synth_settings.mode = MODE_ORIGINAL;
		return;

	}else if(FUNC_SUCCESS == cureMidiCompareSysEx(hybrid_mode, sizeof(hybrid_mode))){

		//Initialize
		cureMidiResetSynth();

		//set hybrid system on
		synth_settings.mode = MODE_HYBRID;
		return;

	}

#endif

}

bool cureMidiIsGetDisplayMessage(char *txt)
{
	switch(synth_settings.disp_msg_type){
		case DSP_NONE:
			return false;
			break;
		case DSP_GM:
			sprintf(txt, "GM System ON");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_GS:
			sprintf(txt, "GS Reset");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_SYSMODE1:
			sprintf(txt, "System Mode1");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_SYSMODE2:
			sprintf(txt, "System Mode2");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_XG:
			sprintf(txt, "XG System ON");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_CURE_ORIG:
			sprintf(txt, "CureSynth Mode1");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_CURE_HYBRID:
			sprintf(txt, "CureSynth Reset");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_BUFFER_FULL:
			sprintf(txt, "Buffer FULL (>_<)");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		case DSP_OVERLOAD:
			sprintf(txt, "Over loaded (>_<)");
			synth_settings.disp_msg_type = DSP_NONE;
			return true;
		default:
			break;
	}

	return false;
}

void cureMidiAnalyzeEvent(uint8_t src_no)
{
	switch(midi_event[src_no].type){

		case MSG_NOTE_ON:
			//When 2nd data_byte is 0x00 in note-on message, perform note off.
			if(0x00 == midi_event[src_no].data_byte[1]){
				//////NOTE OFF
				// if Hold-On is activated, ignore note off message and store ignored note to list.
				// if Sostenuto-On is activated, set note off except sostenuto-note-list.
				if(master.channel[midi_event[src_no].channel].isHoldOn){
					master.channel[midi_event[src_no].channel].hold_note_list[midi_event[src_no].data_byte[0]] = true;

				}else if(master.channel[midi_event[src_no].channel].isSostenutoOn){
					if(STN_INIT == master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]]){
						cureMidiSetNoteOff(midi_event[src_no].channel, midi_event[src_no].data_byte[0]);
					}
					if(STN_REGISTERED == master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]]){
						master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]] = STN_CONTINUE;
					}


				}else{
					cureMidiSetNoteOff(midi_event[src_no].channel, midi_event[src_no].data_byte[0]);
				}
			}else{
				//////NOTE ON
				// if Hold-On is activated, erase ignored note list.
				if(master.channel[midi_event[src_no].channel].isHoldOn){
					master.channel[midi_event[src_no].channel].hold_note_list[midi_event[src_no].data_byte[0]] = false;
				}
				cureMidiSetNoteOn(midi_event[src_no].channel, midi_event[src_no].data_byte[0], midi_event[src_no].data_byte[1]);

				//if already in sostenuto_note_list, initialize.
				master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]] = STN_INIT;
			}

			break;

		case MSG_NOTE_OFF:
			// if Hold-On is activated, ignore note off message and store ignored note to list.
			// if Sostenuto-On is activated, set note off except sostenuto-note-list.
			if(master.channel[midi_event[src_no].channel].isHoldOn){
				master.channel[midi_event[src_no].channel].hold_note_list[midi_event[src_no].data_byte[0]] = true;

			}else if(master.channel[midi_event[src_no].channel].isSostenutoOn){
				if(STN_INIT == master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]]){
					cureMidiSetNoteOff(midi_event[src_no].channel, midi_event[src_no].data_byte[0]);
				}
				if(STN_REGISTERED == master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]]){
					master.channel[midi_event[src_no].channel].sostenuto_note_list[midi_event[src_no].data_byte[0]] = STN_CONTINUE;
				}


			}else{
				cureMidiSetNoteOff(midi_event[src_no].channel, midi_event[src_no].data_byte[0]);
			}
			break;

		case MSG_PITCH:
			cureMidiSetPitchBend(midi_event[src_no].channel, midi_event[src_no].data_byte[0], midi_event[src_no].data_byte[1]);
			break;

		case MSG_CC:
			cureMidiSetControlChange(midi_event[src_no].channel, midi_event[src_no].data_byte[0], midi_event[src_no].data_byte[1]);
			break;

		case MSG_PROG:
			cureMidiSetProgramChange(midi_event[src_no].channel, midi_event[src_no].data_byte[0]);
			break;

		case MSG_SYSEX:
			cureMidiSetSystemExclusive(&(analyzed_status[src_no]), &(midi_event[src_no]));
			break;

		default:
			break;
	}
}

void cureMidiDisplayDataInit(DispInfo* dp)
{
	for(uint32_t i=0; i<CHNUM; i++){
		dp->display_data[i] = 0x00;
	}

	dp->voice = 0;

	dp->voice_is_max = false;
	dp->volume_is_clipping = false;

	for(uint32_t i=0; i<CHNUM; i++){
		dp->note_stat_upper64[i] = 0;
		dp->note_stat_lower64[i] = 0;
	}

}

void cureMidiGetDisplayData(DispInfo* dp)
{
	uint32_t i;
	uint32_t buf;
	float buf_for_pcm;
	uint32_t dispdat[CHNUM];



//making bargraph data 0-15ch
	for(i=0; i<CHNUM; i++){
		dispdat[i] = 0x00;
	}

#define BARGRAPH_PCM_OFFSET_LOW (0.3f)
#define BARGRAPH_PCM_OFFSET_HIGH (0.2f)

	for(i=0; i<OPNUM; i++){
		if( CHNUM > au_operator[i].ch){
//			buf = ((uint8_t)au_operator[i].env.out * au_operator[i].velocity) >> 7 ;
//			buf = ((uint8_t)au_operator[i].velocity) << 1 ;
//			buf = ((!!(uint8_t)au_operator[i].env.out) * au_operator[i].velocity) ;

			if( (PCM == au_operator[i].wav.type) ){
				buf_for_pcm = (float)HI20BIT(au_operator[i].wav.pointer) / ((float)(pcm_len[au_operator[i].wavenum]));
				if(BARGRAPH_PCM_OFFSET_LOW >= buf_for_pcm){
					buf = (uint32_t)((float)au_operator[i].velocity);
				}else if((1.0f - BARGRAPH_PCM_OFFSET_HIGH) <= buf_for_pcm){
					buf = 0;
				}else{
					buf_for_pcm = 1.0f - BARGRAPH_PCM_OFFSET_LOW - BARGRAPH_PCM_OFFSET_HIGH - ((buf_for_pcm - BARGRAPH_PCM_OFFSET_LOW) / (1.0-BARGRAPH_PCM_OFFSET_HIGH));
					buf = (uint32_t)(buf_for_pcm * (float)au_operator[i].velocity);
				}
				buf = __USAT(buf, 7);
				if(dispdat[au_operator[i].ch] < buf){
					dispdat[au_operator[i].ch] = buf;
				}
			}else{
				buf = ((uint8_t)au_operator[i].env.out * au_operator[i].velocity) >> 6;
				buf = __USAT(buf, 7);
				if(dispdat[au_operator[i].ch] < buf){
					dispdat[au_operator[i].ch] = buf;
				}
			}


		}

	}

	for(i=0; i<CHNUM; i++){

		dispdat[i] = (dispdat[i] * synth_settings.expression[i] * synth_settings.volume[i]) >> 15;
		dp->display_data[i] = dispdat[i];
	}



// collect voice data

	static uint8_t voicenum=0;
	static uint32_t voice_draw_counter = 0;
	if(4 <= voice_draw_counter){
		voicenum = synth_voice_number_status.voice_num;
		voice_draw_counter = 0;
		dp->voice_is_max = synth_voice_number_status.isFullyVoiceDetected;
		synth_voice_number_status.isFullyVoiceDetected = false;

		//add volume clipping flag
		dp->volume_is_clipping = synth_is_clipped;
		synth_is_clipped = false;

	}
	voice_draw_counter++;

	dp->voice = voicenum;


// get note on/off status
	for(uint32_t idx_ch=0; idx_ch < CHNUM; idx_ch++){
		for(uint32_t idx_notenum=0; idx_notenum < 128; idx_notenum++){
			if( OPERATOR_OFF != master.channel[idx_ch].operator_to_note.stat[idx_notenum] ){

				if(idx_notenum >= 64){
					dp->note_stat_upper64[idx_ch] |= ((uint64_t)1 << (idx_notenum-64));
				}else{
					dp->note_stat_lower64[idx_ch] |= ((uint64_t)1 << (idx_notenum));
				}

			}else{

				if(idx_notenum >= 64){
					dp->note_stat_upper64[idx_ch] &= ~((uint64_t)1 << (idx_notenum-64));
				}else{
					dp->note_stat_lower64[idx_ch] &= ~((uint64_t)1 << (idx_notenum));
				}

			}
		}
	}

}

void cureMidiGetDisplayChangeInfo(DispSettingChangeInfo* dpc_to_curedraw)
{
//	__disable_irq();
	__HAL_TIM_DISABLE_IT(&htim16, TIM_IT_UPDATE);
	*dpc_to_curedraw = midi_ui_setting_info;
	dpChangeInfoInit(&midi_ui_setting_info);
	__HAL_TIM_ENABLE_IT(&htim16, TIM_IT_UPDATE);
//	__enable_irq();
}


bool curemidi_is_overload_flg = false;

void cureMidiEnableOverloadFlag()
{
	curemidi_is_overload_flg = true;
}

void cureMidiDisableOverloadFlag()
{
	curemidi_is_overload_flg = false;
}

bool cureMidiGetOverloadFlag()
{
	return curemidi_is_overload_flg;
}

bool cureMidiIsDebugDisplay()
{
	return is_debug_displaying;
}
