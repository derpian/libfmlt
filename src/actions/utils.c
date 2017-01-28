/* utils.c generated by valac 0.26.1, the Vala compiler
 * generated from utils.vala, do not modify */

/*      utils.vala*/
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

#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))



gchar* utils_key_file_get_string (GKeyFile* kf, const gchar* group, const gchar* key, const gchar* def_val);
gchar** utils_key_file_get_string_list (GKeyFile* kf, const gchar* group, const gchar* key, gchar** def_val, int def_val_length1, int* result_length1);
static gchar** _vala_array_dup1 (gchar** self, int length);
gchar* utils_key_file_get_locale_string (GKeyFile* kf, const gchar* group, const gchar* key, const gchar* def_val);
gboolean utils_key_file_get_bool (GKeyFile* kf, const gchar* group, const gchar* key, gboolean def_val);
static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);


gchar* utils_key_file_get_string (GKeyFile* kf, const gchar* group, const gchar* key, const gchar* def_val) {
	gchar* result = NULL;
	gchar* val = NULL;
	GError * _inner_error_ = NULL;
	g_return_val_if_fail (kf != NULL, NULL);
	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	{
		gchar* _tmp0_ = NULL;
		GKeyFile* _tmp1_ = NULL;
		const gchar* _tmp2_ = NULL;
		const gchar* _tmp3_ = NULL;
		gchar* _tmp4_ = NULL;
		gchar* _tmp5_ = NULL;
		_tmp1_ = kf;
		_tmp2_ = group;
		_tmp3_ = key;
		_tmp4_ = g_key_file_get_string (_tmp1_, _tmp2_, _tmp3_, &_inner_error_);
		_tmp0_ = _tmp4_;
		if (G_UNLIKELY (_inner_error_ != NULL)) {
			if (_inner_error_->domain == G_KEY_FILE_ERROR) {
				goto __catch6_g_key_file_error;
			}
			_g_free0 (val);
			g_critical ("file %s: line %d: unexpected error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
			g_clear_error (&_inner_error_);
			return NULL;
		}
		_tmp5_ = _tmp0_;
		_tmp0_ = NULL;
		_g_free0 (val);
		val = _tmp5_;
		_g_free0 (_tmp0_);
	}
	goto __finally6;
	__catch6_g_key_file_error:
	{
		GError* err = NULL;
		const gchar* _tmp6_ = NULL;
		gchar* _tmp7_ = NULL;
		err = _inner_error_;
		_inner_error_ = NULL;
		_tmp6_ = def_val;
		_tmp7_ = g_strdup (_tmp6_);
		_g_free0 (val);
		val = _tmp7_;
		_g_error_free0 (err);
	}
	__finally6:
	if (G_UNLIKELY (_inner_error_ != NULL)) {
		_g_free0 (val);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return NULL;
	}
	result = val;
	return result;
}


static gchar** _vala_array_dup1 (gchar** self, int length) {
	gchar** result;
	int i;
	result = g_new0 (gchar*, length + 1);
	for (i = 0; i < length; i++) {
		gchar* _tmp0_ = NULL;
		_tmp0_ = g_strdup (self[i]);
		result[i] = _tmp0_;
	}
	return result;
}


gchar** utils_key_file_get_string_list (GKeyFile* kf, const gchar* group, const gchar* key, gchar** def_val, int def_val_length1, int* result_length1) {
	gchar** result = NULL;
	gchar** val = NULL;
	gint val_length1 = 0;
	gint _val_size_ = 0;
	gchar** _tmp9_ = NULL;
	gint _tmp9__length1 = 0;
	GError * _inner_error_ = NULL;
	g_return_val_if_fail (kf != NULL, NULL);
	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	{
		gchar** _tmp0_ = NULL;
		GKeyFile* _tmp1_ = NULL;
		const gchar* _tmp2_ = NULL;
		const gchar* _tmp3_ = NULL;
		gsize _tmp4_;
		gchar** _tmp5_ = NULL;
		gint _tmp0__length1 = 0;
		gint __tmp0__size_ = 0;
		gchar** _tmp6_ = NULL;
		gint _tmp6__length1 = 0;
		_tmp1_ = kf;
		_tmp2_ = group;
		_tmp3_ = key;
		_tmp5_ = g_key_file_get_string_list (_tmp1_, _tmp2_, _tmp3_, &_tmp4_, &_inner_error_);
		_tmp0_ = _tmp5_;
		_tmp0__length1 = _tmp4_;
		__tmp0__size_ = _tmp0__length1;
		if (G_UNLIKELY (_inner_error_ != NULL)) {
			if (_inner_error_->domain == G_KEY_FILE_ERROR) {
				goto __catch7_g_key_file_error;
			}
			val = (_vala_array_free (val, val_length1, (GDestroyNotify) g_free), NULL);
			g_critical ("file %s: line %d: unexpected error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
			g_clear_error (&_inner_error_);
			return NULL;
		}
		_tmp6_ = _tmp0_;
		_tmp6__length1 = _tmp0__length1;
		_tmp0_ = NULL;
		_tmp0__length1 = 0;
		val = (_vala_array_free (val, val_length1, (GDestroyNotify) g_free), NULL);
		val = _tmp6_;
		val_length1 = _tmp6__length1;
		_val_size_ = val_length1;
		_tmp0_ = (_vala_array_free (_tmp0_, _tmp0__length1, (GDestroyNotify) g_free), NULL);
	}
	goto __finally7;
	__catch7_g_key_file_error:
	{
		GError* err = NULL;
		gchar** _tmp7_ = NULL;
		gint _tmp7__length1 = 0;
		gchar** _tmp8_ = NULL;
		gint _tmp8__length1 = 0;
		err = _inner_error_;
		_inner_error_ = NULL;
		_tmp7_ = def_val;
		_tmp7__length1 = def_val_length1;
		_tmp8_ = (_tmp7_ != NULL) ? _vala_array_dup1 (_tmp7_, _tmp7__length1) : ((gpointer) _tmp7_);
		_tmp8__length1 = _tmp7__length1;
		val = (_vala_array_free (val, val_length1, (GDestroyNotify) g_free), NULL);
		val = _tmp8_;
		val_length1 = _tmp8__length1;
		_val_size_ = val_length1;
		_g_error_free0 (err);
	}
	__finally7:
	if (G_UNLIKELY (_inner_error_ != NULL)) {
		val = (_vala_array_free (val, val_length1, (GDestroyNotify) g_free), NULL);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return NULL;
	}
	_tmp9_ = val;
	_tmp9__length1 = val_length1;
	if (result_length1) {
		*result_length1 = _tmp9__length1;
	}
	result = _tmp9_;
	return result;
}


gchar* utils_key_file_get_locale_string (GKeyFile* kf, const gchar* group, const gchar* key, const gchar* def_val) {
	gchar* result = NULL;
	gchar* val = NULL;
	GError * _inner_error_ = NULL;
	g_return_val_if_fail (kf != NULL, NULL);
	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	{
		gchar* _tmp0_ = NULL;
		GKeyFile* _tmp1_ = NULL;
		const gchar* _tmp2_ = NULL;
		const gchar* _tmp3_ = NULL;
		gchar* _tmp4_ = NULL;
		gchar* _tmp5_ = NULL;
		_tmp1_ = kf;
		_tmp2_ = group;
		_tmp3_ = key;
		_tmp4_ = g_key_file_get_locale_string (_tmp1_, _tmp2_, _tmp3_, NULL, &_inner_error_);
		_tmp0_ = _tmp4_;
		if (G_UNLIKELY (_inner_error_ != NULL)) {
			if (_inner_error_->domain == G_KEY_FILE_ERROR) {
				goto __catch8_g_key_file_error;
			}
			_g_free0 (val);
			g_critical ("file %s: line %d: unexpected error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
			g_clear_error (&_inner_error_);
			return NULL;
		}
		_tmp5_ = _tmp0_;
		_tmp0_ = NULL;
		_g_free0 (val);
		val = _tmp5_;
		_g_free0 (_tmp0_);
	}
	goto __finally8;
	__catch8_g_key_file_error:
	{
		GError* err = NULL;
		const gchar* _tmp6_ = NULL;
		gchar* _tmp7_ = NULL;
		err = _inner_error_;
		_inner_error_ = NULL;
		_tmp6_ = def_val;
		_tmp7_ = g_strdup (_tmp6_);
		_g_free0 (val);
		val = _tmp7_;
		_g_error_free0 (err);
	}
	__finally8:
	if (G_UNLIKELY (_inner_error_ != NULL)) {
		_g_free0 (val);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return NULL;
	}
	result = val;
	return result;
}


gboolean utils_key_file_get_bool (GKeyFile* kf, const gchar* group, const gchar* key, gboolean def_val) {
	gboolean result = FALSE;
	gboolean val = FALSE;
	GError * _inner_error_ = NULL;
	g_return_val_if_fail (kf != NULL, FALSE);
	g_return_val_if_fail (group != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	{
		gboolean _tmp0_ = FALSE;
		GKeyFile* _tmp1_ = NULL;
		const gchar* _tmp2_ = NULL;
		const gchar* _tmp3_ = NULL;
		gboolean _tmp4_ = FALSE;
		_tmp1_ = kf;
		_tmp2_ = group;
		_tmp3_ = key;
		_tmp4_ = g_key_file_get_boolean (_tmp1_, _tmp2_, _tmp3_, &_inner_error_);
		_tmp0_ = _tmp4_;
		if (G_UNLIKELY (_inner_error_ != NULL)) {
			if (_inner_error_->domain == G_KEY_FILE_ERROR) {
				goto __catch9_g_key_file_error;
			}
			g_critical ("file %s: line %d: unexpected error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
			g_clear_error (&_inner_error_);
			return FALSE;
		}
		val = _tmp0_;
	}
	goto __finally9;
	__catch9_g_key_file_error:
	{
		GError* err = NULL;
		gboolean _tmp5_ = FALSE;
		err = _inner_error_;
		_inner_error_ = NULL;
		_tmp5_ = def_val;
		val = _tmp5_;
		_g_error_free0 (err);
	}
	__finally9:
	if (G_UNLIKELY (_inner_error_ != NULL)) {
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return FALSE;
	}
	result = val;
	return result;
}


static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if ((array != NULL) && (destroy_func != NULL)) {
		int i;
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) array)[i] != NULL) {
				destroy_func (((gpointer*) array)[i]);
			}
		}
	}
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	_vala_array_destroy (array, array_length, destroy_func);
	g_free (array);
}



