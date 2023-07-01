#pragma once

#include <cstdint>
#include <array>
#include <cwchar>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// �e�L�X�g�I�u�W�F�N�g�C�X�N���v�g����C�J�����X�N���v�g����̃G�f�B�b�g�{�b�N�X��
// ����� ID �Ɋ���U����:
//	ID = id_edit_base + idx_filter * id_diff_per_filter + id_ofs_****
// ID �����R���g���[���Əd�����邱�Ƃ͂��邪�C���ꂪ�����Ŗ��m�Ȍ듮��������o���͂��̃v���O�C���ł͍��̂Ƃ���Ȃ��D
inline bool editbox_check_id(uint32_t id) {
	static constexpr uint32_t id_edit_base = 22100, id_diff_per_filter = 0x100,
		id_ofs_text = 1, id_ofs_script = 0;
	return id >= id_edit_base &&
		(id - id_edit_base) % id_diff_per_filter <= std::max(id_ofs_text, id_ofs_script);
}
// �E�B���h�E�N���X�̖��O��v���m�F����֐��D���� MAX_PATH �������m�ۂ��Ȃ��Ă��悢�D
template<size_t N>
inline bool check_window_classname(HWND hwnd, const wchar_t(&name)[N])
{
	wchar_t buff[N + 1];
	return ::GetClassNameW(hwnd, buff, std::size(buff)) == N - 1 && std::wcscmp(buff, name) == 0;
}
// �ΏۃG�f�B�b�g�{�b�N�X�̃X�^�C���m�F���ځD
// �����s�\���CEnter�L�[�ŉ��s�\�C�B��Ă��Ȃ��Ƃ��������𖞂������̂Ƃ���D
inline bool editbox_check_style(HWND hwnd)
{
	constexpr auto
		style = ES_MULTILINE | ES_WANTRETURN | WS_VISIBLE,
		style_mask = style;
	return (::GetWindowLongW(hwnd, GWL_STYLE) & style_mask) == style;
}
