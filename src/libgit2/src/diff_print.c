#include "diff_file.h"
#include "patch_generate.h"


	const char *old_prefix;
	const char *new_prefix;
	uint32_t flags;
	int id_strlen;

	int (*strcomp)(const char *, const char *);
	if (!pi->id_strlen) {
			pi->id_strlen = GIT_ABBREV_DEFAULT;
		else if (git_repository__cvar(&pi->id_strlen, repo, GIT_CVAR_ABBREV) < 0)
	if (pi->id_strlen > GIT_OID_HEXSZ)
		pi->id_strlen = GIT_OID_HEXSZ;
		pi->id_strlen = diff->opts.id_abbrev;
		pi->old_prefix = diff->opts.old_prefix;
		pi->new_prefix = diff->opts.new_prefix;

		pi->strcomp = diff->strcomp;
	pi->id_strlen = patch->diff_opts.id_abbrev;
	pi->old_prefix = patch->diff_opts.old_prefix;
	pi->new_prefix = patch->diff_opts.new_prefix;
	return diff_print_info_init__common(pi, out, patch->repo, format, cb, payload);
	int(*strcomp)(const char *, const char *) = pi->strcomp ?
		pi->strcomp : git__strcmp;
	int id_abbrev;
	id_abbrev = delta->old_file.mode ? delta->old_file.id_abbrev :
		delta->new_file.id_abbrev;

	if (pi->id_strlen > id_abbrev) {
		giterr_set(GITERR_PATCH,
			"The patch input contains %d id characters (cannot print %d)",
			id_abbrev, pi->id_strlen);
		return -1;
	}

	git_oid_tostr(start_oid, pi->id_strlen + 1, &delta->old_file.id);
	git_oid_tostr(end_oid, pi->id_strlen + 1, &delta->new_file.id);
		out, (pi->id_strlen <= GIT_OID_HEXSZ) ?
static int diff_print_modes(
	git_buf *out, const git_diff_delta *delta)
{
	git_buf_printf(out, "old mode %o\n", delta->old_file.mode);
	git_buf_printf(out, "new mode %o\n", delta->new_file.mode);

	return git_buf_oom(out) ? -1 : 0;
}

	git_buf *out, const git_diff_delta *delta, int id_strlen)
	if (delta->old_file.mode &&
			id_strlen > delta->old_file.id_abbrev) {
		giterr_set(GITERR_PATCH,
			"The patch input contains %d id characters (cannot print %d)",
			delta->old_file.id_abbrev, id_strlen);
		return -1;
	}

	if ((delta->new_file.mode &&
			id_strlen > delta->new_file.id_abbrev)) {
		giterr_set(GITERR_PATCH,
			"The patch input contains %d id characters (cannot print %d)",
			delta->new_file.id_abbrev, id_strlen);
		return -1;
	}

	git_oid_tostr(start_oid, id_strlen + 1, &delta->old_file.id);
	git_oid_tostr(end_oid, id_strlen + 1, &delta->new_file.id);
		if (delta->old_file.mode == 0)
		else if (delta->new_file.mode == 0)
		else
			diff_print_modes(out, delta);

static int diff_delta_format_path(
	git_buf *out, const char *prefix, const char *filename)
{
	if (git_buf_joinpath(out, prefix, filename) < 0)
		return -1;

	return git_buf_quote(out);
}

	const char *template,
	const char *oldpath,
	const char *newpath)
	if (git_oid_iszero(&delta->old_file.id))

	if (git_oid_iszero(&delta->new_file.id))

	return git_buf_printf(out, template, oldpath, newpath);
}

int diff_delta_format_similarity_header(
	git_buf *out,
	const git_diff_delta *delta)
{
	git_buf old_path = GIT_BUF_INIT, new_path = GIT_BUF_INIT;
	const char *type;
	int error = 0;

	if (delta->similarity > 100) {
		giterr_set(GITERR_PATCH, "invalid similarity %d", delta->similarity);
		error = -1;
		goto done;
	if (delta->status == GIT_DELTA_RENAMED)
		type = "rename";
	else if (delta->status == GIT_DELTA_COPIED)
		type = "copy";
	else
		abort();

	if ((error = git_buf_puts(&old_path, delta->old_file.path)) < 0 ||
		(error = git_buf_puts(&new_path, delta->new_file.path)) < 0 ||
		(error = git_buf_quote(&old_path)) < 0 ||
		(error = git_buf_quote(&new_path)) < 0)
		goto done;

	git_buf_printf(out,
		"similarity index %d%%\n"
		"%s from %s\n"
		"%s to %s\n",
		delta->similarity,
		type, old_path.ptr,
		type, new_path.ptr);

	if (git_buf_oom(out))
		error = -1;

done:
	git_buf_free(&old_path);
	git_buf_free(&new_path);

	return error;
}

static bool delta_is_unchanged(const git_diff_delta *delta)
{
	if (git_oid_iszero(&delta->old_file.id) &&
		git_oid_iszero(&delta->new_file.id))
		return true;

	if (delta->old_file.mode == GIT_FILEMODE_COMMIT ||
		delta->new_file.mode == GIT_FILEMODE_COMMIT)
		return false;

	if (git_oid_equal(&delta->old_file.id, &delta->new_file.id))
		return true;

	return false;
	int id_strlen)
	git_buf old_path = GIT_BUF_INIT, new_path = GIT_BUF_INIT;
	bool unchanged = delta_is_unchanged(delta);
	int error = 0;

	if (!id_strlen)
		id_strlen = GIT_ABBREV_DEFAULT;

	if ((error = diff_delta_format_path(
			&old_path, oldpfx, delta->old_file.path)) < 0 ||
		(error = diff_delta_format_path(
			&new_path, newpfx, delta->new_file.path)) < 0)
		goto done;
	git_buf_printf(out, "diff --git %s %s\n",
		old_path.ptr, new_path.ptr);
	if (delta->status == GIT_DELTA_RENAMED ||
		(delta->status == GIT_DELTA_COPIED && unchanged)) {
		if ((error = diff_delta_format_similarity_header(out, delta)) < 0)
			goto done;
	}
	if (!unchanged) {
		if ((error = diff_print_oid_range(out, delta, id_strlen)) < 0)
			goto done;
		if ((delta->flags & GIT_DIFF_FLAG_BINARY) == 0)
			diff_delta_format_with_paths(out, delta,
				"--- %s\n+++ %s\n", old_path.ptr, new_path.ptr);
	}

	if (unchanged && delta->old_file.mode != delta->new_file.mode)
		diff_print_modes(out, delta);

	if (git_buf_oom(out))
		error = -1;

done:
	git_buf_free(&old_path);
	git_buf_free(&new_path);

	return error;
static int diff_print_patch_file_binary_noshow(
	diff_print_info *pi, git_diff_delta *delta,
	const char *old_pfx, const char *new_pfx)
	git_buf old_path = GIT_BUF_INIT, new_path = GIT_BUF_INIT;
	if ((error = diff_delta_format_path(
			&old_path, old_pfx, delta->old_file.path)) < 0 ||
		(error = diff_delta_format_path(
			&new_path, new_pfx, delta->new_file.path)) < 0)
		goto done;
	pi->line.num_lines = 1;
	error = diff_delta_format_with_paths(
		pi->buf, delta, "Binary files %s and %s differ\n",
		old_path.ptr, new_path.ptr);
done:
	git_buf_free(&old_path);
	git_buf_free(&new_path);
	return error;
		return diff_print_patch_file_binary_noshow(
			pi, delta, old_pfx, new_pfx);
	if (binary->new_file.datalen == 0 && binary->old_file.datalen == 0)
		return 0;

			return diff_print_patch_file_binary_noshow(
				pi, delta, old_pfx, new_pfx);
		pi->old_prefix ? pi->old_prefix : DIFF_OLD_PREFIX_DEFAULT;
		pi->new_prefix ? pi->new_prefix : DIFF_NEW_PREFIX_DEFAULT;
	int id_strlen = binary && show_binary ?
		GIT_OID_HEXSZ : pi->id_strlen;
			pi->buf, delta, oldpfx, newpfx, id_strlen)) < 0)
		pi->old_prefix ? pi->old_prefix : DIFF_OLD_PREFIX_DEFAULT;
		pi->new_prefix ? pi->new_prefix : DIFF_NEW_PREFIX_DEFAULT;
/* print a git_diff to a git_buf */
int git_diff_to_buf(git_buf *out, git_diff *diff, git_diff_format_t format)
{
	assert(out && diff);
	git_buf_sanitize(out);
	return git_diff_print(
		diff, format, git_diff_print_callback__to_buf, out);
}

/* print a git_patch to an output callback */
int git_patch_print(
	git_patch *patch,
	git_diff_line_cb print_cb,
	void *payload)
{
	int error;
	git_buf temp = GIT_BUF_INIT;
	diff_print_info pi;

	assert(patch && print_cb);

	if (!(error = diff_print_info_init_frompatch(
		&pi, &temp, patch,
		GIT_DIFF_FORMAT_PATCH, print_cb, payload)))
	{
		error = git_patch__invoke_callbacks(
			patch,
			diff_print_patch_file, diff_print_patch_binary,
			diff_print_patch_hunk, diff_print_patch_line,
			&pi);

		if (error) /* make sure error message is set */
			giterr_set_after_callback_function(error, "git_patch_print");
	}

	git_buf_free(&temp);

	return error;
}
