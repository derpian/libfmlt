/* parameters.c generated by valac 0.26.1, the Vala compiler
 * generated from parameters.vala, do not modify */

/*      parameters.vala*/
/*      */
/*      Copyright 2011 Hong Jen Yee (PCMan) <pcman.tw@pcman.tw@gmail.com>*/
/*      */
/*      This program is free software; you can redistribute it and/or modify*/
/*      it under the terms of the GNU General Public License as published by*/
/*      the Free Software Foundation; either version 2 of the License, or*/
/*      (at your option) any later version.*/
/*      */
/*      This program is distributed in the hope that it will be useful,*/
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of*/
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the*/
/*      GNU General Public License for more details.*/
/*      */
/*      You should have received a copy of the GNU General Public License*/
/*      along with this program; if not, write to the Free Software*/
/*      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,*/
/*      MA 02110-1301, USA.*/
/*      */
/*      */

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <fm-file-info.h>
#include <fm-path.h>
#include <fm-mime-type.h>

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_string_free0(var) ((var == NULL) ? NULL : (var = (g_string_free (var, TRUE), NULL)))



gchar* fm_file_action_parameters_expand (const gchar* templ, GList* files, gboolean for_display, FmFileInfo* first_file);
gboolean fm_file_action_parameters_is_plural (const gchar* exec);


static gchar string_get (const gchar* self, glong index) {
	gchar result = '\0';
	glong _tmp0_ = 0L;
	gchar _tmp1_ = '\0';
	g_return_val_if_fail (self != NULL, '\0');
	_tmp0_ = index;
	_tmp1_ = ((gchar*) self)[_tmp0_];
	result = _tmp1_;
	return result;
}


static gint string_last_index_of_char (const gchar* self, gunichar c, gint start_index) {
	gint result = 0;
	gchar* _result_ = NULL;
	gint _tmp0_ = 0;
	gunichar _tmp1_ = 0U;
	gchar* _tmp2_ = NULL;
	gchar* _tmp3_ = NULL;
	g_return_val_if_fail (self != NULL, 0);
	_tmp0_ = start_index;
	_tmp1_ = c;
	_tmp2_ = g_utf8_strrchr (((gchar*) self) + _tmp0_, (gssize) (-1), _tmp1_);
	_result_ = _tmp2_;
	_tmp3_ = _result_;
	if (_tmp3_ != NULL) {
		gchar* _tmp4_ = NULL;
		_tmp4_ = _result_;
		result = (gint) (_tmp4_ - ((gchar*) self));
		return result;
	} else {
		result = -1;
		return result;
	}
}


static gchar* string_slice (const gchar* self, glong start, glong end) {
	gchar* result = NULL;
	glong string_length = 0L;
	gint _tmp0_ = 0;
	gint _tmp1_ = 0;
	glong _tmp2_ = 0L;
	glong _tmp5_ = 0L;
	gboolean _tmp8_ = FALSE;
	glong _tmp9_ = 0L;
	gboolean _tmp12_ = FALSE;
	glong _tmp13_ = 0L;
	glong _tmp16_ = 0L;
	glong _tmp17_ = 0L;
	glong _tmp18_ = 0L;
	glong _tmp19_ = 0L;
	glong _tmp20_ = 0L;
	gchar* _tmp21_ = NULL;
	g_return_val_if_fail (self != NULL, NULL);
	_tmp0_ = strlen (self);
	_tmp1_ = _tmp0_;
	string_length = (glong) _tmp1_;
	_tmp2_ = start;
	if (_tmp2_ < ((glong) 0)) {
		glong _tmp3_ = 0L;
		glong _tmp4_ = 0L;
		_tmp3_ = string_length;
		_tmp4_ = start;
		start = _tmp3_ + _tmp4_;
	}
	_tmp5_ = end;
	if (_tmp5_ < ((glong) 0)) {
		glong _tmp6_ = 0L;
		glong _tmp7_ = 0L;
		_tmp6_ = string_length;
		_tmp7_ = end;
		end = _tmp6_ + _tmp7_;
	}
	_tmp9_ = start;
	if (_tmp9_ >= ((glong) 0)) {
		glong _tmp10_ = 0L;
		glong _tmp11_ = 0L;
		_tmp10_ = start;
		_tmp11_ = string_length;
		_tmp8_ = _tmp10_ <= _tmp11_;
	} else {
		_tmp8_ = FALSE;
	}
	g_return_val_if_fail (_tmp8_, NULL);
	_tmp13_ = end;
	if (_tmp13_ >= ((glong) 0)) {
		glong _tmp14_ = 0L;
		glong _tmp15_ = 0L;
		_tmp14_ = end;
		_tmp15_ = string_length;
		_tmp12_ = _tmp14_ <= _tmp15_;
	} else {
		_tmp12_ = FALSE;
	}
	g_return_val_if_fail (_tmp12_, NULL);
	_tmp16_ = start;
	_tmp17_ = end;
	g_return_val_if_fail (_tmp16_ <= _tmp17_, NULL);
	_tmp18_ = start;
	_tmp19_ = end;
	_tmp20_ = start;
	_tmp21_ = g_strndup (((gchar*) self) + _tmp18_, (gsize) (_tmp19_ - _tmp20_));
	result = _tmp21_;
	return result;
}


gchar* fm_file_action_parameters_expand (const gchar* templ, GList* files, gboolean for_display, FmFileInfo* first_file) {
	gchar* result = NULL;
	const gchar* _tmp0_ = NULL;
	GString* _result_ = NULL;
	GString* _tmp1_ = NULL;
	gint len = 0;
	const gchar* _tmp2_ = NULL;
	gint _tmp3_ = 0;
	gint _tmp4_ = 0;
	FmFileInfo* _tmp5_ = NULL;
	GString* _tmp244_ = NULL;
	gchar* _tmp245_ = NULL;
	_tmp0_ = templ;
	if (_tmp0_ == NULL) {
		result = NULL;
		return result;
	}
	_tmp1_ = g_string_new ("");
	_result_ = _tmp1_;
	_tmp2_ = templ;
	_tmp3_ = strlen (_tmp2_);
	_tmp4_ = _tmp3_;
	len = _tmp4_;
	_tmp5_ = first_file;
	if (_tmp5_ == NULL) {
		GList* _tmp6_ = NULL;
		GList* _tmp7_ = NULL;
		gconstpointer _tmp8_ = NULL;
		_tmp6_ = files;
		_tmp7_ = g_list_first (_tmp6_);
		_tmp8_ = _tmp7_->data;
		first_file = (FmFileInfo*) _tmp8_;
	}
	{
		gint i = 0;
		i = 0;
		{
			gboolean _tmp9_ = FALSE;
			_tmp9_ = TRUE;
			while (TRUE) {
				gint _tmp11_ = 0;
				gint _tmp12_ = 0;
				gchar ch = '\0';
				const gchar* _tmp13_ = NULL;
				gint _tmp14_ = 0;
				gchar _tmp15_ = '\0';
				gchar _tmp16_ = '\0';
				if (!_tmp9_) {
					gint _tmp10_ = 0;
					_tmp10_ = i;
					i = _tmp10_ + 1;
				}
				_tmp9_ = FALSE;
				_tmp11_ = i;
				_tmp12_ = len;
				if (!(_tmp11_ < _tmp12_)) {
					break;
				}
				_tmp13_ = templ;
				_tmp14_ = i;
				_tmp15_ = string_get (_tmp13_, (glong) _tmp14_);
				ch = _tmp15_;
				_tmp16_ = ch;
				if (_tmp16_ == '%') {
					gint _tmp17_ = 0;
					const gchar* _tmp18_ = NULL;
					gint _tmp19_ = 0;
					gchar _tmp20_ = '\0';
					gchar _tmp21_ = '\0';
					_tmp17_ = i;
					i = _tmp17_ + 1;
					_tmp18_ = templ;
					_tmp19_ = i;
					_tmp20_ = string_get (_tmp18_, (glong) _tmp19_);
					ch = _tmp20_;
					_tmp21_ = ch;
					switch (_tmp21_) {
						case 'b':
						{
							gboolean _tmp22_ = FALSE;
							_tmp22_ = for_display;
							if (_tmp22_) {
								GString* _tmp23_ = NULL;
								FmFileInfo* _tmp24_ = NULL;
								const gchar* _tmp25_ = NULL;
								gchar* _tmp26_ = NULL;
								gchar* _tmp27_ = NULL;
								_tmp23_ = _result_;
								_tmp24_ = first_file;
								_tmp25_ = fm_file_info_get_name (_tmp24_);
								_tmp26_ = g_filename_display_name (_tmp25_);
								_tmp27_ = _tmp26_;
								g_string_append (_tmp23_, _tmp27_);
								_g_free0 (_tmp27_);
							} else {
								GString* _tmp28_ = NULL;
								FmFileInfo* _tmp29_ = NULL;
								const gchar* _tmp30_ = NULL;
								gchar* _tmp31_ = NULL;
								gchar* _tmp32_ = NULL;
								_tmp28_ = _result_;
								_tmp29_ = first_file;
								_tmp30_ = fm_file_info_get_name (_tmp29_);
								_tmp31_ = g_shell_quote (_tmp30_);
								_tmp32_ = _tmp31_;
								g_string_append (_tmp28_, _tmp32_);
								_g_free0 (_tmp32_);
							}
							break;
						}
						case 'B':
						{
							GList* _tmp33_ = NULL;
							GString* _tmp48_ = NULL;
							const gchar* _tmp49_ = NULL;
							GString* _tmp50_ = NULL;
							gssize _tmp51_ = 0L;
							gchar _tmp52_ = '\0';
							_tmp33_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp33_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										gboolean _tmp34_ = FALSE;
										GString* _tmp47_ = NULL;
										_tmp34_ = for_display;
										if (_tmp34_) {
											GString* _tmp35_ = NULL;
											FmFileInfo* _tmp36_ = NULL;
											const gchar* _tmp37_ = NULL;
											gchar* _tmp38_ = NULL;
											gchar* _tmp39_ = NULL;
											gchar* _tmp40_ = NULL;
											gchar* _tmp41_ = NULL;
											_tmp35_ = _result_;
											_tmp36_ = fi;
											_tmp37_ = fm_file_info_get_name (_tmp36_);
											_tmp38_ = g_filename_display_name (_tmp37_);
											_tmp39_ = _tmp38_;
											_tmp40_ = g_shell_quote (_tmp39_);
											_tmp41_ = _tmp40_;
											g_string_append (_tmp35_, _tmp41_);
											_g_free0 (_tmp41_);
											_g_free0 (_tmp39_);
										} else {
											GString* _tmp42_ = NULL;
											FmFileInfo* _tmp43_ = NULL;
											const gchar* _tmp44_ = NULL;
											gchar* _tmp45_ = NULL;
											gchar* _tmp46_ = NULL;
											_tmp42_ = _result_;
											_tmp43_ = fi;
											_tmp44_ = fm_file_info_get_name (_tmp43_);
											_tmp45_ = g_shell_quote (_tmp44_);
											_tmp46_ = _tmp45_;
											g_string_append (_tmp42_, _tmp46_);
											_g_free0 (_tmp46_);
										}
										_tmp47_ = _result_;
										g_string_append_c (_tmp47_, ' ');
									}
								}
							}
							_tmp48_ = _result_;
							_tmp49_ = _tmp48_->str;
							_tmp50_ = _result_;
							_tmp51_ = _tmp50_->len;
							_tmp52_ = string_get (_tmp49_, (glong) (_tmp51_ - 1));
							if (_tmp52_ == ' ') {
								GString* _tmp53_ = NULL;
								GString* _tmp54_ = NULL;
								gssize _tmp55_ = 0L;
								_tmp53_ = _result_;
								_tmp54_ = _result_;
								_tmp55_ = _tmp54_->len;
								g_string_truncate (_tmp53_, (gsize) (_tmp55_ - 1));
							}
							break;
						}
						case 'c':
						{
							GString* _tmp56_ = NULL;
							GList* _tmp57_ = NULL;
							guint _tmp58_ = 0U;
							_tmp56_ = _result_;
							_tmp57_ = files;
							_tmp58_ = g_list_length (_tmp57_);
							g_string_append_printf (_tmp56_, "%u", _tmp58_);
							break;
						}
						case 'd':
						{
							FmPath* base_dir = NULL;
							FmFileInfo* _tmp59_ = NULL;
							FmPath* _tmp60_ = NULL;
							FmPath* _tmp61_ = NULL;
							gchar* str = NULL;
							FmPath* _tmp62_ = NULL;
							gchar* _tmp63_ = NULL;
							gboolean _tmp64_ = FALSE;
							GString* _tmp67_ = NULL;
							const gchar* _tmp68_ = NULL;
							gchar* _tmp69_ = NULL;
							gchar* _tmp70_ = NULL;
							_tmp59_ = first_file;
							_tmp60_ = fm_file_info_get_path (_tmp59_);
							_tmp61_ = fm_path_get_parent (_tmp60_);
							base_dir = _tmp61_;
							_tmp62_ = base_dir;
							_tmp63_ = fm_path_to_str (_tmp62_);
							str = _tmp63_;
							_tmp64_ = for_display;
							if (_tmp64_) {
								const gchar* _tmp65_ = NULL;
								gchar* _tmp66_ = NULL;
								_tmp65_ = str;
								_tmp66_ = g_filename_display_name (_tmp65_);
								_g_free0 (str);
								str = _tmp66_;
							}
							_tmp67_ = _result_;
							_tmp68_ = str;
							_tmp69_ = g_shell_quote (_tmp68_);
							_tmp70_ = _tmp69_;
							g_string_append (_tmp67_, _tmp70_);
							_g_free0 (_tmp70_);
							_g_free0 (str);
							break;
						}
						case 'D':
						{
							GList* _tmp71_ = NULL;
							GString* _tmp85_ = NULL;
							const gchar* _tmp86_ = NULL;
							GString* _tmp87_ = NULL;
							gssize _tmp88_ = 0L;
							gchar _tmp89_ = '\0';
							_tmp71_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp71_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										FmPath* base_dir = NULL;
										FmFileInfo* _tmp72_ = NULL;
										FmPath* _tmp73_ = NULL;
										FmPath* _tmp74_ = NULL;
										gchar* str = NULL;
										FmPath* _tmp75_ = NULL;
										gchar* _tmp76_ = NULL;
										gboolean _tmp77_ = FALSE;
										GString* _tmp80_ = NULL;
										const gchar* _tmp81_ = NULL;
										gchar* _tmp82_ = NULL;
										gchar* _tmp83_ = NULL;
										GString* _tmp84_ = NULL;
										_tmp72_ = fi;
										_tmp73_ = fm_file_info_get_path (_tmp72_);
										_tmp74_ = fm_path_get_parent (_tmp73_);
										base_dir = _tmp74_;
										_tmp75_ = base_dir;
										_tmp76_ = fm_path_to_str (_tmp75_);
										str = _tmp76_;
										_tmp77_ = for_display;
										if (_tmp77_) {
											const gchar* _tmp78_ = NULL;
											gchar* _tmp79_ = NULL;
											_tmp78_ = str;
											_tmp79_ = g_filename_display_name (_tmp78_);
											_g_free0 (str);
											str = _tmp79_;
										}
										_tmp80_ = _result_;
										_tmp81_ = str;
										_tmp82_ = g_shell_quote (_tmp81_);
										_tmp83_ = _tmp82_;
										g_string_append (_tmp80_, _tmp83_);
										_g_free0 (_tmp83_);
										_tmp84_ = _result_;
										g_string_append_c (_tmp84_, ' ');
										_g_free0 (str);
									}
								}
							}
							_tmp85_ = _result_;
							_tmp86_ = _tmp85_->str;
							_tmp87_ = _result_;
							_tmp88_ = _tmp87_->len;
							_tmp89_ = string_get (_tmp86_, (glong) (_tmp88_ - 1));
							if (_tmp89_ == ' ') {
								GString* _tmp90_ = NULL;
								GString* _tmp91_ = NULL;
								gssize _tmp92_ = 0L;
								_tmp90_ = _result_;
								_tmp91_ = _result_;
								_tmp92_ = _tmp91_->len;
								g_string_truncate (_tmp90_, (gsize) (_tmp92_ - 1));
							}
							break;
						}
						case 'f':
						{
							gchar* filename = NULL;
							FmFileInfo* _tmp93_ = NULL;
							FmPath* _tmp94_ = NULL;
							gchar* _tmp95_ = NULL;
							gboolean _tmp96_ = FALSE;
							GString* _tmp99_ = NULL;
							const gchar* _tmp100_ = NULL;
							gchar* _tmp101_ = NULL;
							gchar* _tmp102_ = NULL;
							_tmp93_ = first_file;
							_tmp94_ = fm_file_info_get_path (_tmp93_);
							_tmp95_ = fm_path_to_str (_tmp94_);
							filename = _tmp95_;
							_tmp96_ = for_display;
							if (_tmp96_) {
								const gchar* _tmp97_ = NULL;
								gchar* _tmp98_ = NULL;
								_tmp97_ = filename;
								_tmp98_ = g_filename_display_name (_tmp97_);
								_g_free0 (filename);
								filename = _tmp98_;
							}
							_tmp99_ = _result_;
							_tmp100_ = filename;
							_tmp101_ = g_shell_quote (_tmp100_);
							_tmp102_ = _tmp101_;
							g_string_append (_tmp99_, _tmp102_);
							_g_free0 (_tmp102_);
							_g_free0 (filename);
							break;
						}
						case 'F':
						{
							GList* _tmp103_ = NULL;
							GString* _tmp115_ = NULL;
							const gchar* _tmp116_ = NULL;
							GString* _tmp117_ = NULL;
							gssize _tmp118_ = 0L;
							gchar _tmp119_ = '\0';
							_tmp103_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp103_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										gchar* filename = NULL;
										FmFileInfo* _tmp104_ = NULL;
										FmPath* _tmp105_ = NULL;
										gchar* _tmp106_ = NULL;
										gboolean _tmp107_ = FALSE;
										GString* _tmp110_ = NULL;
										const gchar* _tmp111_ = NULL;
										gchar* _tmp112_ = NULL;
										gchar* _tmp113_ = NULL;
										GString* _tmp114_ = NULL;
										_tmp104_ = fi;
										_tmp105_ = fm_file_info_get_path (_tmp104_);
										_tmp106_ = fm_path_to_str (_tmp105_);
										filename = _tmp106_;
										_tmp107_ = for_display;
										if (_tmp107_) {
											const gchar* _tmp108_ = NULL;
											gchar* _tmp109_ = NULL;
											_tmp108_ = filename;
											_tmp109_ = g_filename_display_name (_tmp108_);
											_g_free0 (filename);
											filename = _tmp109_;
										}
										_tmp110_ = _result_;
										_tmp111_ = filename;
										_tmp112_ = g_shell_quote (_tmp111_);
										_tmp113_ = _tmp112_;
										g_string_append (_tmp110_, _tmp113_);
										_g_free0 (_tmp113_);
										_tmp114_ = _result_;
										g_string_append_c (_tmp114_, ' ');
										_g_free0 (filename);
									}
								}
							}
							_tmp115_ = _result_;
							_tmp116_ = _tmp115_->str;
							_tmp117_ = _result_;
							_tmp118_ = _tmp117_->len;
							_tmp119_ = string_get (_tmp116_, (glong) (_tmp118_ - 1));
							if (_tmp119_ == ' ') {
								GString* _tmp120_ = NULL;
								GString* _tmp121_ = NULL;
								gssize _tmp122_ = 0L;
								_tmp120_ = _result_;
								_tmp121_ = _result_;
								_tmp122_ = _tmp121_->len;
								g_string_truncate (_tmp120_, (gsize) (_tmp122_ - 1));
							}
							break;
						}
						case 'h':
						{
							GString* _tmp123_ = NULL;
							const gchar* _tmp124_ = NULL;
							_tmp123_ = _result_;
							_tmp124_ = g_get_host_name ();
							g_string_append (_tmp123_, _tmp124_);
							break;
						}
						case 'm':
						{
							FmFileInfo* fi = NULL;
							GList* _tmp125_ = NULL;
							GList* _tmp126_ = NULL;
							gconstpointer _tmp127_ = NULL;
							GString* _tmp128_ = NULL;
							FmFileInfo* _tmp129_ = NULL;
							FmMimeType* _tmp130_ = NULL;
							const gchar* _tmp131_ = NULL;
							_tmp125_ = files;
							_tmp126_ = g_list_first (_tmp125_);
							_tmp127_ = _tmp126_->data;
							fi = (FmFileInfo*) _tmp127_;
							_tmp128_ = _result_;
							_tmp129_ = fi;
							_tmp130_ = fm_file_info_get_mime_type (_tmp129_);
							_tmp131_ = fm_mime_type_get_type (_tmp130_);
							g_string_append (_tmp128_, _tmp131_);
							break;
						}
						case 'M':
						{
							GList* _tmp132_ = NULL;
							_tmp132_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp132_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										GString* _tmp133_ = NULL;
										FmFileInfo* _tmp134_ = NULL;
										FmMimeType* _tmp135_ = NULL;
										const gchar* _tmp136_ = NULL;
										GString* _tmp137_ = NULL;
										_tmp133_ = _result_;
										_tmp134_ = fi;
										_tmp135_ = fm_file_info_get_mime_type (_tmp134_);
										_tmp136_ = fm_mime_type_get_type (_tmp135_);
										g_string_append (_tmp133_, _tmp136_);
										_tmp137_ = _result_;
										g_string_append_c (_tmp137_, ' ');
									}
								}
							}
							break;
						}
						case 'n':
						{
							GString* _tmp138_ = NULL;
							const gchar* _tmp139_ = NULL;
							_tmp138_ = _result_;
							_tmp139_ = g_get_user_name ();
							g_string_append (_tmp138_, _tmp139_);
							break;
						}
						case 'o':
						case 'O':
						{
							break;
						}
						case 'p':
						{
							break;
						}
						case 's':
						{
							gchar* uri = NULL;
							FmFileInfo* _tmp140_ = NULL;
							FmPath* _tmp141_ = NULL;
							gchar* _tmp142_ = NULL;
							GString* _tmp143_ = NULL;
							const gchar* _tmp144_ = NULL;
							gchar* _tmp145_ = NULL;
							gchar* _tmp146_ = NULL;
							_tmp140_ = first_file;
							_tmp141_ = fm_file_info_get_path (_tmp140_);
							_tmp142_ = fm_path_to_uri (_tmp141_);
							uri = _tmp142_;
							_tmp143_ = _result_;
							_tmp144_ = uri;
							_tmp145_ = g_uri_parse_scheme (_tmp144_);
							_tmp146_ = _tmp145_;
							g_string_append (_tmp143_, _tmp146_);
							_g_free0 (_tmp146_);
							_g_free0 (uri);
							break;
						}
						case 'u':
						{
							GString* _tmp147_ = NULL;
							FmFileInfo* _tmp148_ = NULL;
							FmPath* _tmp149_ = NULL;
							gchar* _tmp150_ = NULL;
							gchar* _tmp151_ = NULL;
							_tmp147_ = _result_;
							_tmp148_ = first_file;
							_tmp149_ = fm_file_info_get_path (_tmp148_);
							_tmp150_ = fm_path_to_uri (_tmp149_);
							_tmp151_ = _tmp150_;
							g_string_append (_tmp147_, _tmp151_);
							_g_free0 (_tmp151_);
							break;
						}
						case 'U':
						{
							GList* _tmp152_ = NULL;
							GString* _tmp159_ = NULL;
							const gchar* _tmp160_ = NULL;
							GString* _tmp161_ = NULL;
							gssize _tmp162_ = 0L;
							gchar _tmp163_ = '\0';
							_tmp152_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp152_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										GString* _tmp153_ = NULL;
										FmFileInfo* _tmp154_ = NULL;
										FmPath* _tmp155_ = NULL;
										gchar* _tmp156_ = NULL;
										gchar* _tmp157_ = NULL;
										GString* _tmp158_ = NULL;
										_tmp153_ = _result_;
										_tmp154_ = fi;
										_tmp155_ = fm_file_info_get_path (_tmp154_);
										_tmp156_ = fm_path_to_uri (_tmp155_);
										_tmp157_ = _tmp156_;
										g_string_append (_tmp153_, _tmp157_);
										_g_free0 (_tmp157_);
										_tmp158_ = _result_;
										g_string_append_c (_tmp158_, ' ');
									}
								}
							}
							_tmp159_ = _result_;
							_tmp160_ = _tmp159_->str;
							_tmp161_ = _result_;
							_tmp162_ = _tmp161_->len;
							_tmp163_ = string_get (_tmp160_, (glong) (_tmp162_ - 1));
							if (_tmp163_ == ' ') {
								GString* _tmp164_ = NULL;
								GString* _tmp165_ = NULL;
								gssize _tmp166_ = 0L;
								_tmp164_ = _result_;
								_tmp165_ = _result_;
								_tmp166_ = _tmp165_->len;
								g_string_truncate (_tmp164_, (gsize) (_tmp166_ - 1));
							}
							break;
						}
						case 'w':
						{
							const gchar* basename = NULL;
							FmFileInfo* _tmp167_ = NULL;
							const gchar* _tmp168_ = NULL;
							gint pos = 0;
							const gchar* _tmp169_ = NULL;
							gint _tmp170_ = 0;
							gint _tmp171_ = 0;
							_tmp167_ = first_file;
							_tmp168_ = fm_file_info_get_name (_tmp167_);
							basename = _tmp168_;
							_tmp169_ = basename;
							_tmp170_ = string_last_index_of_char (_tmp169_, (gunichar) '.', 0);
							pos = _tmp170_;
							_tmp171_ = pos;
							if (_tmp171_ == (-1)) {
								GString* _tmp172_ = NULL;
								const gchar* _tmp173_ = NULL;
								gchar* _tmp174_ = NULL;
								gchar* _tmp175_ = NULL;
								_tmp172_ = _result_;
								_tmp173_ = basename;
								_tmp174_ = g_shell_quote (_tmp173_);
								_tmp175_ = _tmp174_;
								g_string_append (_tmp172_, _tmp175_);
								_g_free0 (_tmp175_);
							} else {
								GString* _tmp176_ = NULL;
								const gchar* _tmp177_ = NULL;
								gint _tmp178_ = 0;
								gchar* _tmp179_ = NULL;
								gchar* _tmp180_ = NULL;
								gchar* _tmp181_ = NULL;
								gchar* _tmp182_ = NULL;
								_tmp176_ = _result_;
								_tmp177_ = basename;
								_tmp178_ = pos;
								_tmp179_ = string_slice (_tmp177_, (glong) 0, (glong) _tmp178_);
								_tmp180_ = _tmp179_;
								_tmp181_ = g_shell_quote (_tmp180_);
								_tmp182_ = _tmp181_;
								g_string_append (_tmp176_, _tmp182_);
								_g_free0 (_tmp182_);
								_g_free0 (_tmp180_);
							}
							break;
						}
						case 'W':
						{
							GList* _tmp183_ = NULL;
							GString* _tmp201_ = NULL;
							const gchar* _tmp202_ = NULL;
							GString* _tmp203_ = NULL;
							gssize _tmp204_ = 0L;
							gchar _tmp205_ = '\0';
							_tmp183_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp183_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										const gchar* basename = NULL;
										FmFileInfo* _tmp184_ = NULL;
										const gchar* _tmp185_ = NULL;
										gint pos = 0;
										const gchar* _tmp186_ = NULL;
										gint _tmp187_ = 0;
										gint _tmp188_ = 0;
										GString* _tmp200_ = NULL;
										_tmp184_ = fi;
										_tmp185_ = fm_file_info_get_name (_tmp184_);
										basename = _tmp185_;
										_tmp186_ = basename;
										_tmp187_ = string_last_index_of_char (_tmp186_, (gunichar) '.', 0);
										pos = _tmp187_;
										_tmp188_ = pos;
										if (_tmp188_ == (-1)) {
											GString* _tmp189_ = NULL;
											const gchar* _tmp190_ = NULL;
											gchar* _tmp191_ = NULL;
											gchar* _tmp192_ = NULL;
											_tmp189_ = _result_;
											_tmp190_ = basename;
											_tmp191_ = g_shell_quote (_tmp190_);
											_tmp192_ = _tmp191_;
											g_string_append (_tmp189_, _tmp192_);
											_g_free0 (_tmp192_);
										} else {
											GString* _tmp193_ = NULL;
											const gchar* _tmp194_ = NULL;
											gint _tmp195_ = 0;
											gchar* _tmp196_ = NULL;
											gchar* _tmp197_ = NULL;
											gchar* _tmp198_ = NULL;
											gchar* _tmp199_ = NULL;
											_tmp193_ = _result_;
											_tmp194_ = basename;
											_tmp195_ = pos;
											_tmp196_ = string_slice (_tmp194_, (glong) 0, (glong) _tmp195_);
											_tmp197_ = _tmp196_;
											_tmp198_ = g_shell_quote (_tmp197_);
											_tmp199_ = _tmp198_;
											g_string_append (_tmp193_, _tmp199_);
											_g_free0 (_tmp199_);
											_g_free0 (_tmp197_);
										}
										_tmp200_ = _result_;
										g_string_append_c (_tmp200_, ' ');
									}
								}
							}
							_tmp201_ = _result_;
							_tmp202_ = _tmp201_->str;
							_tmp203_ = _result_;
							_tmp204_ = _tmp203_->len;
							_tmp205_ = string_get (_tmp202_, (glong) (_tmp204_ - 1));
							if (_tmp205_ == ' ') {
								GString* _tmp206_ = NULL;
								GString* _tmp207_ = NULL;
								gssize _tmp208_ = 0L;
								_tmp206_ = _result_;
								_tmp207_ = _result_;
								_tmp208_ = _tmp207_->len;
								g_string_truncate (_tmp206_, (gsize) (_tmp208_ - 1));
							}
							break;
						}
						case 'x':
						{
							const gchar* basename = NULL;
							FmFileInfo* _tmp209_ = NULL;
							const gchar* _tmp210_ = NULL;
							gint pos = 0;
							const gchar* _tmp211_ = NULL;
							gint _tmp212_ = 0;
							const gchar* ext = NULL;
							gint _tmp213_ = 0;
							GString* _tmp216_ = NULL;
							const gchar* _tmp217_ = NULL;
							gchar* _tmp218_ = NULL;
							gchar* _tmp219_ = NULL;
							_tmp209_ = first_file;
							_tmp210_ = fm_file_info_get_name (_tmp209_);
							basename = _tmp210_;
							_tmp211_ = basename;
							_tmp212_ = string_last_index_of_char (_tmp211_, (gunichar) '.', 0);
							pos = _tmp212_;
							_tmp213_ = pos;
							if (_tmp213_ >= 0) {
								const gchar* _tmp214_ = NULL;
								gint _tmp215_ = 0;
								_tmp214_ = basename;
								_tmp215_ = pos;
								ext = (const gchar*) ((((guint8*) _tmp214_) + _tmp215_) + 1);
							} else {
								ext = "";
							}
							_tmp216_ = _result_;
							_tmp217_ = ext;
							_tmp218_ = g_shell_quote (_tmp217_);
							_tmp219_ = _tmp218_;
							g_string_append (_tmp216_, _tmp219_);
							_g_free0 (_tmp219_);
							break;
						}
						case 'X':
						{
							GList* _tmp220_ = NULL;
							GString* _tmp233_ = NULL;
							const gchar* _tmp234_ = NULL;
							GString* _tmp235_ = NULL;
							gssize _tmp236_ = 0L;
							gchar _tmp237_ = '\0';
							_tmp220_ = files;
							{
								GList* fi_collection = NULL;
								GList* fi_it = NULL;
								fi_collection = _tmp220_;
								for (fi_it = fi_collection; fi_it != NULL; fi_it = fi_it->next) {
									FmFileInfo* fi = NULL;
									fi = (FmFileInfo*) fi_it->data;
									{
										const gchar* basename = NULL;
										FmFileInfo* _tmp221_ = NULL;
										const gchar* _tmp222_ = NULL;
										gint pos = 0;
										const gchar* _tmp223_ = NULL;
										gint _tmp224_ = 0;
										const gchar* ext = NULL;
										gint _tmp225_ = 0;
										GString* _tmp228_ = NULL;
										const gchar* _tmp229_ = NULL;
										gchar* _tmp230_ = NULL;
										gchar* _tmp231_ = NULL;
										GString* _tmp232_ = NULL;
										_tmp221_ = fi;
										_tmp222_ = fm_file_info_get_name (_tmp221_);
										basename = _tmp222_;
										_tmp223_ = basename;
										_tmp224_ = string_last_index_of_char (_tmp223_, (gunichar) '.', 0);
										pos = _tmp224_;
										_tmp225_ = pos;
										if (_tmp225_ >= 0) {
											const gchar* _tmp226_ = NULL;
											gint _tmp227_ = 0;
											_tmp226_ = basename;
											_tmp227_ = pos;
											ext = (const gchar*) ((((guint8*) _tmp226_) + _tmp227_) + 1);
										} else {
											ext = "";
										}
										_tmp228_ = _result_;
										_tmp229_ = ext;
										_tmp230_ = g_shell_quote (_tmp229_);
										_tmp231_ = _tmp230_;
										g_string_append (_tmp228_, _tmp231_);
										_g_free0 (_tmp231_);
										_tmp232_ = _result_;
										g_string_append_c (_tmp232_, ' ');
									}
								}
							}
							_tmp233_ = _result_;
							_tmp234_ = _tmp233_->str;
							_tmp235_ = _result_;
							_tmp236_ = _tmp235_->len;
							_tmp237_ = string_get (_tmp234_, (glong) (_tmp236_ - 1));
							if (_tmp237_ == ' ') {
								GString* _tmp238_ = NULL;
								GString* _tmp239_ = NULL;
								gssize _tmp240_ = 0L;
								_tmp238_ = _result_;
								_tmp239_ = _result_;
								_tmp240_ = _tmp239_->len;
								g_string_truncate (_tmp238_, (gsize) (_tmp240_ - 1));
							}
							break;
						}
						case '%':
						{
							GString* _tmp241_ = NULL;
							_tmp241_ = _result_;
							g_string_append_c (_tmp241_, '%');
							break;
						}
						case '\0':
						{
							break;
						}
						default:
						break;
					}
				} else {
					GString* _tmp242_ = NULL;
					gchar _tmp243_ = '\0';
					_tmp242_ = _result_;
					_tmp243_ = ch;
					g_string_append_c (_tmp242_, _tmp243_);
				}
			}
		}
	}
	_tmp244_ = _result_;
	_tmp245_ = _tmp244_->str;
	_tmp244_->str = NULL;
	result = _tmp245_;
	_g_string_free0 (_result_);
	return result;
}


gboolean fm_file_action_parameters_is_plural (const gchar* exec) {
	gboolean result = FALSE;
	const gchar* _tmp0_ = NULL;
	GString* _result_ = NULL;
	GString* _tmp1_ = NULL;
	gint len = 0;
	const gchar* _tmp2_ = NULL;
	gint _tmp3_ = 0;
	gint _tmp4_ = 0;
	_tmp0_ = exec;
	if (_tmp0_ == NULL) {
		result = FALSE;
		return result;
	}
	_tmp1_ = g_string_new ("");
	_result_ = _tmp1_;
	_tmp2_ = exec;
	_tmp3_ = strlen (_tmp2_);
	_tmp4_ = _tmp3_;
	len = _tmp4_;
	{
		gint i = 0;
		i = 0;
		{
			gboolean _tmp5_ = FALSE;
			_tmp5_ = TRUE;
			while (TRUE) {
				gint _tmp7_ = 0;
				gint _tmp8_ = 0;
				gchar ch = '\0';
				const gchar* _tmp9_ = NULL;
				gint _tmp10_ = 0;
				gchar _tmp11_ = '\0';
				gchar _tmp12_ = '\0';
				if (!_tmp5_) {
					gint _tmp6_ = 0;
					_tmp6_ = i;
					i = _tmp6_ + 1;
				}
				_tmp5_ = FALSE;
				_tmp7_ = i;
				_tmp8_ = len;
				if (!(_tmp7_ < _tmp8_)) {
					break;
				}
				_tmp9_ = exec;
				_tmp10_ = i;
				_tmp11_ = string_get (_tmp9_, (glong) _tmp10_);
				ch = _tmp11_;
				_tmp12_ = ch;
				if (_tmp12_ == '%') {
					gint _tmp13_ = 0;
					const gchar* _tmp14_ = NULL;
					gint _tmp15_ = 0;
					gchar _tmp16_ = '\0';
					gchar _tmp17_ = '\0';
					_tmp13_ = i;
					i = _tmp13_ + 1;
					_tmp14_ = exec;
					_tmp15_ = i;
					_tmp16_ = string_get (_tmp14_, (glong) _tmp15_);
					ch = _tmp16_;
					_tmp17_ = ch;
					switch (_tmp17_) {
						case 'B':
						case 'D':
						case 'F':
						case 'M':
						case 'O':
						case 'U':
						case 'W':
						case 'X':
						{
							result = TRUE;
							_g_string_free0 (_result_);
							return result;
						}
						case 'b':
						case 'd':
						case 'f':
						case 'm':
						case 'o':
						case 'u':
						case 'w':
						case 'x':
						{
							result = FALSE;
							_g_string_free0 (_result_);
							return result;
						}
						default:
						{
							break;
						}
					}
				}
			}
		}
	}
	result = FALSE;
	_g_string_free0 (_result_);
	return result;
}



