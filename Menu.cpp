#include "Features.h"
#include "GUI/gui.h"
#include "javascript/js_loader.h"

#include "menu_icons/configs.h"
#include "menu_icons/legit.h"
#include "menu_icons/misc.h"
#include "menu_icons/profile.h"
#include "menu_icons/rage.h"
#include "menu_icons/skins.h"
#include "menu_icons/visuals.h"

#include "ragebot_icons/item_assaultsuit.h"
#include "ragebot_icons/weapon_awp.h"
#include "ragebot_icons/weapon_deagle.h"
#include "ragebot_icons/weapon_glock.h"
#include "ragebot_icons/weapon_m4a1.h"
#include "ragebot_icons/weapon_scar20.h"
#include "ragebot_icons/weapon_ssg08.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define FCVAR_HIDDEN			(1<<4)	// Hidden. Doesn't appear in find or 
#define FCVAR_UNREGISTERED		(1<<0)	// If this is set, don't add to linked list, etc.
#define FCVAR_DEVELOPMENTONLY	(1<<1)	// Hidden in released products. Flag is removed 

vector<string> ConfigList;


BOOL SearchFiles(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders)
{
	LPTSTR part;
	char tmp[MAX_PATH];
	char name[MAX_PATH];

	HANDLE hSearch = NULL;
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

	if (bInnerFolders)
	{
		if (GetFullPathName(lpszFileName, MAX_PATH, tmp, &part) == 0) return FALSE;
		strcpy(name, part);
		strcpy(part, str("*.*"));
		wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		if (!((hSearch = FindFirstFile(tmp, &wfd)) == INVALID_HANDLE_VALUE))
			do
			{
				if (!strncmp(wfd.cFileName, str("."), 1) || !strncmp(wfd.cFileName, str(".."), 2))
					continue;

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char next[MAX_PATH];
					if (GetFullPathName(lpszFileName, MAX_PATH, next, &part) == 0) return FALSE;
					strcpy(part, wfd.cFileName);
					strcat(next, str("\\"));
					strcat(next, name);

					SearchFiles(next, lpSearchFunc, TRUE);
				}
			} while (FindNextFile(hSearch, &wfd));
			FindClose(hSearch);
	}

	if ((hSearch = FindFirstFile(lpszFileName, &wfd)) == INVALID_HANDLE_VALUE)
		return TRUE;
	do
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file[MAX_PATH];
			if (GetFullPathName(lpszFileName, MAX_PATH, file, &part) == 0) return FALSE;
			strcpy(part, wfd.cFileName);

			lpSearchFunc(wfd.cFileName);
		}
	while (FindNextFile(hSearch, &wfd));
	FindClose(hSearch);
	return TRUE;
}
void ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}

void RefreshConfigs()
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		ConfigList.clear();
		string ConfigDir = std::string(path) + str("\\weave\\configs\\");
		CreateDirectory(ConfigDir.c_str(), 0);
		SearchFiles((ConfigDir + str("*.cfg")).c_str(), ReadConfigs, FALSE);
	}
}
void EnableHiddenCVars()
{
	auto p = **reinterpret_cast<ConCommandBase***>(interfaces.cvars + 0x34);
	for (auto c = p->m_pNext; c != nullptr; c = c->m_pNext)
	{
		ConCommandBase* cmd = c;
		cmd->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
		cmd->m_nFlags &= ~FCVAR_HIDDEN;
	}
}

Vector2D g_mouse;
namespace menu_colors {
	color_t main_color;
	color_t background1;
	color_t background2;
}
bool enable_rage() { return vars.ragebot.enable; };
bool enable_antiaim() { return vars.antiaim.enable; };
bool enable_esp() { return vars.visuals.enable; };

void CMenu::draw_indicators()
{
}

LPDIRECT3DTEXTURE9 ragebot_icons[7];
LPDIRECT3DTEXTURE9 tabs_icons[7];
color_t main_color;

bool override_default() {
	return vars.ragebot.enable && (vars.ragebot.active_index == 0 || !vars.ragebot.weapon[vars.ragebot.active_index].enable);
}
/*
namespace ragebot_tab
{
	c_child* groupboxes = nullptr;
	c_child* misc_esp_main = nullptr;
	int current_subtab = 0;

	void init_misc_esp_main() {
		if (current_subtab > 4) {
			misc_esp_main->add_element(new c_text(str("AIMBOT misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

			vars.ragebot.active_index = current_subtab - 5;

			misc_esp_main->add_element(new c_checkbox(str("Override default misc_esp_main"), &vars.ragebot.weapon[vars.ragebot.active_index].enable,
				[]() { return enable_rage() && vars.ragebot.active_index > 0; }));

			misc_esp_main->add_element(new c_slider(str("Hitchance"), &vars.ragebot.weapon[vars.ragebot.active_index].hitchance, 0, 100,
				override_default));

			misc_esp_main->add_element(new c_slider(str("Doubletap HC"), &vars.ragebot.weapon[vars.ragebot.active_index].doubletap_hc, 0, 100,
				[]() {
					return override_default()
						&& (g_Binds[bind_double_tap].key > 0 || g_Binds[bind_double_tap].active)
						&& vars.ragebot.active_index != 2
						&& vars.ragebot.active_index != 3;
				}));

			misc_esp_main->add_element(new c_slider(str("Minimum damage"), &vars.ragebot.weapon[vars.ragebot.active_index].mindamage, 0, 120,
				override_default));
			
			misc_esp_main->add_element(new c_slider(str("Override damage"), &vars.ragebot.weapon[vars.ragebot.active_index].mindamage_override, 0, 120,
				[]() { return override_default() && g_Binds[bind_override_dmg].key > 0; }));

			misc_esp_main->add_element(new c_separator([]() { return enable_rage() && 
				(vars.ragebot.weapon[vars.ragebot.active_index].enable
				|| vars.ragebot.active_index == 0); }));

			misc_esp_main->add_element(new c_checkbox(str("Auto stop"), &vars.ragebot.weapon[vars.ragebot.active_index].quickstop,
				[]() { return override_default(); }));

			misc_esp_main->add_element(new c_multicombo(str("Options"), &vars.ragebot.weapon[vars.ragebot.active_index].quickstop_options, {
				str("Between shots"),
				str("Force accuracy")
				},
				[]() {
					return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].quickstop;
				}));

			misc_esp_main->add_element(new c_separator([]() { return enable_rage() &&
				(vars.ragebot.weapon[vars.ragebot.active_index].enable
					|| vars.ragebot.active_index == 0); }));

			misc_esp_main->add_element(new c_multicombo(str("Hitboxes"), &vars.ragebot.weapon[vars.ragebot.active_index].hitscan, {
					str("Head"),
					str("Upper chest"),
					str("Chest"),
					str("Lower chest"),
					str("Pelvis"),
					str("Stomach"),
					str("Legs"),
					str("Feet"),
					str("Arms"),

				}, override_default));

			misc_esp_main->add_element(new c_multicombo(str("Prefer safepoint"), &vars.ragebot.weapon[vars.ragebot.active_index].prefer_safepoint, {
				str("Head"),
				str("Body"),
				str("Limbs"),
			}, override_default));

			misc_esp_main->add_element(new c_multicombo(str("Force safepoint"), &vars.ragebot.weapon[vars.ragebot.active_index].force_safepoint, {
				str("Head"),
				str("Body"),
				str("Limbs"),
			}, override_default));

			misc_esp_main->add_element(new c_slider(str("Max misses"), &vars.ragebot.weapon[vars.ragebot.active_index].max_misses, 0, 5,
				[]() { return override_default(); }));

			misc_esp_main->add_element(new c_checkbox(str("Custom point scale"), &vars.ragebot.weapon[vars.ragebot.active_index].static_scale,
				[]() { return override_default(); }));

			misc_esp_main->add_element(new c_slider(str("Point scale head"), &vars.ragebot.weapon[vars.ragebot.active_index].scale_head, 0, 100,
				[]() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].static_scale; }));

			misc_esp_main->add_element(new c_slider(str("Point scale body"), &vars.ragebot.weapon[vars.ragebot.active_index].scale_body, 0, 100,
				[]() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].static_scale; }));

			misc_esp_main->add_element(new c_text(str("Please toggle aimbot before setup!"), 0, []() { return !enable_rage(); }));
		}
		else
		{
			switch (current_subtab)
			{
			case 0:
			{
				misc_esp_main->add_element(new c_text(str("MAIN misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.ragebot.enable));

				misc_esp_main->add_element(new c_checkbox(str("Silent aim"),
					&vars.ragebot.silent, enable_rage));

				misc_esp_main->add_element(new c_checkbox(str("Auto fire"),
					&vars.ragebot.autoshoot, enable_rage));

				misc_esp_main->add_element(new c_combo(str("Auto scope"), &vars.ragebot.autoscope, {
					str("Off"), str("Always"), str("Hitchance fail") }, enable_rage));

				misc_esp_main->add_element(new c_slider(str("FOV"), &vars.ragebot.fov,
					0.f, 180.f, enable_rage));

				misc_esp_main->add_element(new c_checkbox(str("Zeus bot"), &vars.ragebot.zeusbot,
					enable_rage));

				misc_esp_main->add_element(new c_slider(str("Zeus hitchance"), &vars.ragebot.zeuschance,
					0.f, 100.f, []() {return enable_rage() && vars.ragebot.zeusbot; }));

				misc_esp_main->add_element(new c_keybind(str("Force safe points"), &g_Binds[bind_force_safepoint], enable_rage));
				misc_esp_main->add_element(new c_keybind(str("Force body aim"), &g_Binds[bind_baim], enable_rage));

				misc_esp_main->add_element(new c_keybind(str("Override damage"),
					&g_Binds[bind_override_dmg], enable_rage));

				misc_esp_main->add_element(new c_checkbox(str("Resolver"), &vars.ragebot.resolver,
					enable_rage));
			}
			break;
			case 1:
			{
				misc_esp_main->add_element(new c_text(str("EXPLOIT misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_keybind(str("Double-tap"), &g_Binds[bind_double_tap], []() {
					return vars.ragebot.enable && vars.misc.restrict_type == 1;
					}));

				misc_esp_main->add_element(new c_checkbox(str("Teleport boost"), &vars.ragebot.dt_teleport, []() {
					return enable_rage() && (g_Binds[bind_double_tap].key > 0 || g_Binds[bind_double_tap].active);
					}));

				misc_esp_main->add_element(new c_keybind(str("Hide-shots"), &g_Binds[bind_hide_shots], []() {
					return vars.ragebot.enable && vars.misc.restrict_type == 1;
					}));

				misc_esp_main->add_element(new c_slider(str("Charge delay"), &vars.ragebot.recharge_time,
					0, 14));


				misc_esp_main->add_element(new c_text(str("Please toggle aimbot before setup!"), 0, []() { return !enable_rage(); }));
			}
			break;
			case 2:
			{
				misc_esp_main->add_element(new c_text(str("ANTI-AIM misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.antiaim.enable));

				misc_esp_main->add_element(new c_checkbox(str("On use"), &vars.antiaim.aa_on_use,
					[]() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_combo(str("Pitch direction"), &vars.antiaim.pitch, {
					str("Disabled"),
					str("Down"),
					str("Ideal down"),
				}, []() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_combo(str("Yaw direction"), &vars.antiaim.yaw, {
					str("Disabled"),
					str("Backwards"),
				}, []() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_separator([]() { return enable_antiaim(); }));
				misc_esp_main->add_element(new c_text(str("DESYNC"), 25, []() { return enable_antiaim(); }, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.antiaim.desync,
					[]() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_combo(str("Type"), &vars.antiaim.desync_direction,
					{
						str("Manual"),
						str("Peek real"),
						str("Peek desync"),
						str("Jitter")
					},
					[]() { return enable_antiaim() && vars.antiaim.desync; }));

				misc_esp_main->add_element(new c_keybind(str("Inverter"), &g_Binds[bind_aa_inverter],
					[]() { return enable_antiaim() && vars.antiaim.desync_direction == 0 && vars.antiaim.desync; }));

				misc_esp_main->add_element(new c_slider(str("Amount"), &vars.antiaim.desync_amount, 0, 100,
					[]() { return enable_antiaim() && vars.antiaim.desync; }));

				misc_esp_main->add_element(new c_separator([]() { return enable_antiaim(); }));
				misc_esp_main->add_element(new c_text(str("YAW misc_esp_main"), 25, []() { return enable_antiaim(); }, color_t(127, 127, 127)));
				misc_esp_main->add_element(new c_slider(str("Jitter angle"), &vars.antiaim.jitter_angle, 0.f, 45.f,
					[]() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_checkbox(str("At target"),
					&vars.antiaim.attarget, []() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_checkbox(str("Off when offscreen"),
					&vars.antiaim.attarget_off_when_offsreen, []() { return enable_antiaim() && vars.antiaim.attarget; }));

				misc_esp_main->add_element(new c_checkbox(str("Manual yaw direction"),
					&vars.antiaim.manual_antiaim, enable_antiaim));

				misc_esp_main->add_element(new c_checkbox(str("Ignore at target"),
					&vars.antiaim.ignore_attarget, []() { return enable_antiaim() && vars.antiaim.manual_antiaim; }));

				misc_esp_main->add_element(new c_keybind(str("Left"), &g_Binds[bind_manual_left], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				misc_esp_main->add_element(new c_keybind(str("Right"), &g_Binds[bind_manual_right], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				misc_esp_main->add_element(new c_keybind(str("Back"), &g_Binds[bind_manual_back], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				misc_esp_main->add_element(new c_keybind(str("Forward"), &g_Binds[bind_manual_forward], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));
			}
			break;
			case 3:
			{
				misc_esp_main->add_element(new c_text(str("FAKE-LAG misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_combo(str("Lag mode"), &vars.antiaim.fakelag,
					{
						str("Disabled"),
						str("Static"),
						str("Fluctuate"),
						str("Adaptive"),
					},
					enable_antiaim));

				misc_esp_main->add_element(new c_separator([]() { return vars.antiaim.enable && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_slider(str("Limit"), &vars.antiaim.fakelagfactor, 1, 62,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_separator([]() { return vars.antiaim.enable && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_checkbox(str("When standing"), &vars.antiaim.fakelag_when_standing,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_checkbox(str("When using exploits"), &vars.antiaim.fakelag_when_exploits,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_checkbox(str("Only on peek"), &vars.antiaim.fakelag_on_peek,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				misc_esp_main->add_element(new c_text(str("Please toggle anti-aims before setup!"), 0, []() { return !vars.antiaim.enable; }));
			}
			break;
			case 4:
			{
				misc_esp_main->add_element(new c_text(str("OTHER"), 25, nullptr, color_t(127, 127, 127)));

				misc_esp_main->add_element(new c_keybind(str("Fake duck"), &g_Binds[bind_fake_duck],
					[]() { return enable_antiaim() && vars.misc.restrict_type == 1; }));

				misc_esp_main->add_element(new c_keybind(str("Slow walk"), &g_Binds[bind_slow_walk],
					[]() { return enable_antiaim(); }));

				misc_esp_main->add_element(new c_text(str("Please toggle anti-aims before setup!"), 0, []() { return !vars.antiaim.enable; }));
			}
			break;
			}
		}

		misc_esp_main->initialize_elements();
	}

	void init()
	{
		groupboxes = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		groupboxes->set_size(Vector2D(192, 552));
		groupboxes->set_position(Vector2D(8, 48));

		groupboxes->set_padding(Vector2D(8, 12));

		auto subtab_selector = new c_listbox("", &current_subtab, {}, 0, true, nullptr, [](int) {
			misc_esp_main->update_state(init_misc_esp_main);
			});
		c_listbox::c_selector s1;
		s1.name = str("RAGEBOT");
		s1.elements = {
			str("Aimbot"), // 0
			str("Exploits"), // 1
			str("Anti-aim"), // 2
			str("Fake-lag"), // 3
			str("Other"), // 4
		};

		c_listbox::c_selector s2;
		s2.name = str("WEAPONS");
		s2.elements = {
			str("Default"),
			str("Auto-sniper"),
			str("SSG-08"),
			str("AWP"),
			str("Rifles"),
			str("Pistols"),
			str("Heavy pistols")
		};

		subtab_selector->make_subtabs();
		subtab_selector->add_selector(s1);
		subtab_selector->add_selector(s2);

		groupboxes->add_element(subtab_selector);

		groupboxes->initialize_elements();
		g_Menu->window->add_element(groupboxes);
		misc_esp_main = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		misc_esp_main->set_size(Vector2D(376, 552));
		misc_esp_main->set_position(Vector2D(208, 48));
		misc_esp_main->set_padding(Vector2D(40, 8));
		init_misc_esp_main();
		g_Menu->window->add_element(misc_esp_main);
		g_Menu->window->set_title(str("Ragebot"));
	}
}
*/










/**
namespace visuals_tab
{
	c_child* groupboxes = nullptr;
	c_child* misc_esp_main = nullptr;
	int current_subtab = 0;
	void init_misc_esp_main() {
		switch (current_subtab)
		{
		case 0:
		{
			misc_esp_main->add_element(new c_text(str("ESP misc_esp_main"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Enable"),
				&vars.visuals.enable));

			misc_esp_main->add_element(new c_checkbox(str("Dormant"),
				&vars.visuals.dormant, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.box_color, false,
				[]() { return enable_esp() && vars.visuals.box; }));

			misc_esp_main->add_element(new c_checkbox(str("Box"),
				&vars.visuals.box, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.name_color, true,
				[]() { return enable_esp() && vars.visuals.name; }));

			misc_esp_main->add_element(new c_checkbox(str("Name"),
				&vars.visuals.name, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.weapon_color, true,
				[]() { return enable_esp() && vars.visuals.weapon; }));

			misc_esp_main->add_element(new c_checkbox(str("Weapon"),
				&vars.visuals.weapon, enable_esp));

			misc_esp_main->add_element(new c_checkbox(str("Health"),
				&vars.visuals.healthbar, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.hp_color, false,
				[]() { return enable_esp() && vars.visuals.healthbar && vars.visuals.override_hp; }));

			misc_esp_main->add_element(new c_checkbox(str("Override health color"),
				&vars.visuals.override_hp, [] { return enable_esp() && vars.visuals.healthbar; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.ammo_color, true,
				[]() { return enable_esp() && vars.visuals.ammo; }));

			misc_esp_main->add_element(new c_checkbox(str("Ammo"),
				&vars.visuals.ammo, enable_esp));

			misc_esp_main->add_element(new c_checkbox(str("Zeus warning"),
				&vars.visuals.zeus_warning, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.out_of_fov_color, true,
				[]() { return enable_esp() && vars.visuals.out_of_fov; }));

			misc_esp_main->add_element(new c_checkbox(str("Out of fov arrow"),
				&vars.visuals.out_of_fov, enable_esp));

			misc_esp_main->add_element(new c_slider(str("Distance"), &vars.visuals.out_of_fov_distance, 0, 300,
				[]() { return enable_esp() && vars.visuals.out_of_fov; }));

			misc_esp_main->add_element(new c_slider(str("Size"), &vars.visuals.out_of_fov_size, 10, 35,
				[]() { return enable_esp() && vars.visuals.out_of_fov; }));

			misc_esp_main->add_element(new c_checkbox(str("Show multipoint"), &vars.visuals.shot_multipoint, enable_esp));

			misc_esp_main->add_element(new c_multicombo(str("Flags"),
				&vars.visuals.flags, {
					str("Armor"),
					str("Scoped"),
					str("Flashed"),
					str("Defuse kit"),
					str("Fake duck"),
					str("Distance"),
					str("Position"),
				}, enable_esp));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.flags_color, true,
				[]() { return enable_esp() && vars.visuals.flags > 0; }));

			misc_esp_main->add_element(new c_text("Flags color", 0, []() { return enable_esp() && vars.visuals.flags > 0; }));
		}
		break;
		case 1:
		{
			misc_esp_main->add_element(new c_text(str("VISIBLE"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.visuals.chams[enemy_visible].enable));
				auto enemy_visible_enabled = []() { return vars.visuals.chams[enemy_visible].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_visible].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, enemy_visible_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].material_color, true, enemy_visible_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, enemy_visible_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].glass_color, true, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].metallic_color, true, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_visible].phong_amount, 0, 100, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_visible].rim_amount, -100, 100, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[enemy_visible].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, enemy_visible_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].glow_color[0], true, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].glow_color[1], true, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].wireframe_color, true, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].overlay & 4;
						}));
				}
			}

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("THROUGH WALLS"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.visuals.chams[enemy_xqz].enable));
				auto enemy_xqz_enabled = []() { return vars.visuals.chams[enemy_xqz].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_xqz].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, enemy_xqz_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].material_color, true, enemy_xqz_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, enemy_xqz_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].glass_color, true, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].metallic_color, true, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_xqz].phong_amount, 0, 100, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_xqz].rim_amount, -100, 100, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
						}));
				
				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[enemy_xqz].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, enemy_xqz_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].glow_color[0], true, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].glow_color[1], true, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].wireframe_color, true, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].overlay & 4;
						}));
				}
			}

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("BACKTRACK"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.visuals.chams[enemy_history].enable,
					[]() { return vars.ragebot.enable || vars.legitbot.enable; }));

				auto enemy_history_enabled = []() { return vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable); };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_history].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, enemy_history_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].material_color, true, enemy_history_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, enemy_history_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].glass_color, true, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].metallic_color, true, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_history].phong_amount, 0, 100, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_history].rim_amount, -100, 100, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[enemy_history].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, enemy_history_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].glow_color[0], true, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].glow_color[1], true, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_history].wireframe_color, true, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[enemy_history].enable && (vars.ragebot.enable || vars.legitbot.enable) && vars.visuals.chams[enemy_history].overlay & 4;
						}));
				}

				misc_esp_main->add_element(new c_text(str("Please toggle rage or legit aimbot before setup!"), 0, []() { return !vars.ragebot.enable && !vars.legitbot.enable; }));
			}

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("SHOT"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.visuals.chams[enemy_ragebot_shot].enable,
					[]() { return vars.ragebot.enable; }));

				auto enemy_ragebot_shot_enabled = []() { return vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable); };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_ragebot_shot].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, enemy_ragebot_shot_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].material_color, true, enemy_ragebot_shot_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, enemy_ragebot_shot_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].glass_color, true, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].metallic_color, true, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_ragebot_shot].phong_amount, 0, 100, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_ragebot_shot].rim_amount, -100, 100, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[enemy_ragebot_shot].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, enemy_ragebot_shot_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].glow_color[0], true, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].glow_color[1], true, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_ragebot_shot].wireframe_color, true, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[enemy_ragebot_shot].enable && (vars.ragebot.enable) && vars.visuals.chams[enemy_ragebot_shot].overlay & 4;
						}));
				}

				misc_esp_main->add_element(new c_text(str("Please toggle rage aimbot before setup!"), 0, []() { return !vars.ragebot.enable; }));
			}
		}
		break;
		case 2:
		{
			misc_esp_main->add_element(new c_text(str("GLOW"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.glow_color, true,
				[]()
				{ return vars.visuals.glow; }));

			misc_esp_main->add_element(new c_checkbox(str("On entity"), &vars.visuals.glow));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.local_glow_clr, true,
				[]()
				{ return vars.visuals.local_glow; }));

			misc_esp_main->add_element(new c_checkbox(str("On local"), &vars.visuals.local_glow));

			misc_esp_main->add_element(new c_combo(str("Glow style"), &vars.visuals.glowtype, {
				str("Normal"),
				str("Pulsating"),
				}));
		}
		break;
		case 3:
		{
			misc_esp_main->add_element(new c_text(str("LOCAL MODEL"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Override"), &vars.visuals.chams[local_default].enable));

				misc_esp_main->add_element(new c_checkbox(str("Transparency in scope"), &vars.visuals.blend_on_scope));
				misc_esp_main->add_element(new c_slider(str("Transparency amt"), &vars.visuals.blend_value, 0, 100,
					[]() { return vars.visuals.blend_on_scope; }));

				auto local_default_enabled = []() { return vars.visuals.chams[local_default].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_default].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, local_default_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].material_color, true, local_default_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, local_default_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].glass_color, true, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].metallic_color, true, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_default].phong_amount, 0, 100, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_default].rim_amount, -100, 100, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[local_default].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, local_default_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].glow_color[0], true, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].glow_color[1], true, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].wireframe_color, true, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].overlay & 4;
						}));
				}
			}
			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("DESYNC MODEL"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Enable"), &vars.visuals.chams[local_desync].enable));
				auto local_desync_enabled = []() { return vars.visuals.chams[local_desync].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_desync].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, local_desync_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].material_color, true, local_desync_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, local_desync_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].glass_color, true, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].metallic_color, true, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_desync].phong_amount, 0, 100, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_desync].rim_amount, -100, 100, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[local_desync].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, local_desync_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].glow_color[0], true, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].glow_color[1], true, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].wireframe_color, true, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].overlay & 4;
						}));
				}
			}
			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("ARMS"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Override"), &vars.visuals.chams[local_arms].enable));
				auto local_arms_enabled = []() { return vars.visuals.chams[local_arms].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_arms].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, local_arms_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].material_color, true, local_arms_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, local_arms_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].glass_color, true, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].metallic_color, true, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_arms].phong_amount, 0, 100, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_arms].rim_amount, -100, 100, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[local_arms].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, local_arms_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].glow_color[0], true, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].glow_color[1], true, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].wireframe_color, true, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].overlay & 4;
						}));
				}
			}
			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("WEAPON"), 25, nullptr, color_t(127, 127, 127)));
			{
				misc_esp_main->add_element(new c_checkbox(str("Override"), &vars.visuals.chams[local_weapon].enable));
				auto local_weapons_enabled = []() { return vars.visuals.chams[local_weapon].enable; };

				misc_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_weapon].material,
					{
						str("Default"),
						str("Flat"),
						str("Glass"),
						str("Metallic")
					}, local_weapons_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].material_color, true, local_weapons_enabled));
				misc_esp_main->add_element(new c_text(str("Material color"), 0, local_weapons_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].glass_color, true, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glass color"), 0, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].metallic_color, true, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
						}));

					misc_esp_main->add_element(new c_text(str("Metallic color"), 0, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_weapon].phong_amount, 0, 100, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
						}));

					misc_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_weapon].rim_amount, -100, 100, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
						}));
				}

				misc_esp_main->add_element(new c_multicombo(str("Overlay"), &vars.visuals.chams[local_weapon].overlay,
					{
						str("Glow fade"),
						str("Glow line"),
						str("Wireframe")
					}, local_weapons_enabled));

				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].glow_color[0], true, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 1;
						}));

					misc_esp_main->add_element(new c_text(str("Glow fade color"), 0, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 1;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].glow_color[1], true, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 2;
						}));

					misc_esp_main->add_element(new c_text(str("Glow line color"), 0, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 2;
						}));
				}
				{
					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].wireframe_color, true, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 4;
						}));

					misc_esp_main->add_element(new c_text(str("Wireframe color"), 0, []() { return
						vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].overlay & 4;
						}));
				}
			}
		}
		break;
		case 4:
			misc_esp_main->add_element(new c_text(str("INDICATORS"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Watermark"), &vars.visuals.watermark));
			misc_esp_main->add_element(new c_checkbox(str("Keybind states"), &vars.visuals.indicators));
			misc_esp_main->add_element(new c_checkbox(str("Indicators"), &vars.visuals.indicators));
			misc_esp_main->add_element(new c_checkbox(str("Ragebot only"), &vars.visuals.indicators_rage,
				[]() { return vars.visuals.indicators; }));

			misc_esp_main->add_element(new c_combo(str("Taser range"), &vars.visuals.taser_range, { str("Off"), str("Rainbow"), str("Default") }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.taser_range_color, true,
				[]() { return vars.visuals.taser_range > 0; }));

			misc_esp_main->add_element(new c_text(str("Taser color"), 0,
				[]() { return vars.visuals.taser_range > 0; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nadepred_color, true,
				[]() { return vars.visuals.nadepred; }));

			misc_esp_main->add_element(new c_checkbox(str("Grenade prediction"),
				&vars.visuals.nadepred));

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("LOGS"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_multicombo(str("Eventlog"), &vars.visuals.eventlog, {
				str("Damage"), str("Aimbot"), str("Missed shots"), str("Misc"),
				}));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.eventlog_color, false,
				[]() { return vars.visuals.eventlog > 0; }));

			misc_esp_main->add_element(new c_text(str("Eventlog color"), 0, []() { return vars.visuals.eventlog > 0; }));

			break;
		case 5:
			misc_esp_main->add_element(new c_text(str("EFFECTS"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Kill effect"),
				&vars.visuals.kill_effect));

			misc_esp_main->add_element(new c_slider(str("Aspect ratio"), &vars.visuals.aspect_ratio, 0, 250));

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("VIEWMODEL"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_slider(str("Viewmodel fov"), &vars.misc.viewmodelfov, 68, 145));

			misc_esp_main->add_element(new c_slider(str("Offset X"), &vars.misc.viewmodel_x, -25.f, 25.f));
			misc_esp_main->add_element(new c_slider(str("Offset Y"), &vars.misc.viewmodel_y, -25.f, 25.f));
			misc_esp_main->add_element(new c_slider(str("Offset Z"), &vars.misc.viewmodel_z, -25.f, 25.f));

			break;
		case 6:
			misc_esp_main->add_element(new c_text(str("OTHER"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_keybind(str("Thirdperson"), &g_Binds[bind_third_person]));

			misc_esp_main->add_element(new c_slider(str("Distance"), &vars.visuals.thirdperson_dist, 0, 300, []() {
				return g_Binds[bind_third_person].key > 0;
				}));

			misc_esp_main->add_element(new c_checkbox(str("Save death notices"),
				&vars.visuals.preverse_killfeed));

			misc_esp_main->add_element(new c_checkbox(str("Force crosshair"),
				&vars.visuals.force_crosshair));

			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("MARKERS"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_multicombo(str("Hitmarker"),
				&vars.visuals.hitmarker, { str("Screen center"), str("World")}));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.hitmarker_color, false, []() { return vars.visuals.hitmarker > 0; }));
			misc_esp_main->add_element(new c_text(str("Hitmarker color", 0, []() { return vars.visuals.hitmarker > 0; })));

			misc_esp_main->add_element(new c_checkbox(str("Show damage"),
				&vars.visuals.visualize_damage, []() {
					return vars.visuals.hitmarker > 0;
				}));

			misc_esp_main->add_element(new c_checkbox(str("Hit sound"),
				&vars.visuals.hitmarker_sound));

			misc_esp_main->add_element(new c_combo(str("Sound type"),
				&vars.visuals.hitmarker_sound_type, { str("Default"), str("Warning"), str("Click"), str("COD") }, []() {
					return vars.visuals.hitmarker_sound;
				}));
			break;
		case 7:
			misc_esp_main->add_element(new c_text(str("WORLD ESP"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.weapons.color, true,
				[]() { return vars.visuals.world.weapons.enabled; }));

			misc_esp_main->add_element(new c_checkbox(str("Show weapons"), &vars.visuals.world.weapons.enabled));
			misc_esp_main->add_element(new c_checkbox(str("Bomb timer"), &vars.visuals.world.weapons.planted_bomb));

			misc_esp_main->add_element(new c_checkbox(str("Show grenades"), &vars.visuals.world.projectiles.enable));

			{
				static const auto& projectiles_enabled = []() {
					return vars.visuals.world.projectiles.enable;
				};
				misc_esp_main->add_element(new c_multicombo(str("Filter"), &vars.visuals.world.projectiles.filter,
					{
						str("Team"),
						str("Enemy"),
						str("Local player")
					}, projectiles_enabled));

				misc_esp_main->add_element(new c_checkbox(str("Trajectories"), &vars.visuals.world.projectiles.trajectories, projectiles_enabled));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[0], false,
					[]() { return vars.visuals.world.projectiles.enable
					&& vars.visuals.world.projectiles.filter & 1; }));

				misc_esp_main->add_element(new c_text(str("Team color"), 0,
					[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 1; }));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[1], false,
					[]() { return vars.visuals.world.projectiles.enable
					&& vars.visuals.world.projectiles.filter & 2; }));

				misc_esp_main->add_element(new c_text(str("Enemy color"), 0,
					[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 2; }));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[2], false,
					[]() { return vars.visuals.world.projectiles.enable
					&& vars.visuals.world.projectiles.filter & 4; }));

				misc_esp_main->add_element(new c_text(str("Local color"), 0,
					[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 4; }));
			}
			break;
		case 8:
			misc_esp_main->add_element(new c_text(str("EFFECTS"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_multicombo(str("Removals"),
				&vars.visuals.remove, {
					str("Visual recoil"), // 1
					str("Smoke"), // 2
					str("Flash"), // 4
					str("Scope"), // 8
					str("Post processing"), // 16
					str("Fog"), // 32
					str("World shadow"), // 64
					str("Foot shadow"), // 128
					str("Viewmodel bob"), // 256
				}));

			misc_esp_main->add_element(new c_slider(str("World fov"), &vars.misc.worldfov, 0, 50));

			misc_esp_main->add_element(new c_slider(str("Zoom fov"), &vars.misc.zoomfov, 0, 100));
			misc_esp_main->add_element(new c_separator());
			misc_esp_main->add_element(new c_text(str("NIGHTMODE"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_checkbox(str("Night mode"), &vars.visuals.nightmode));

			misc_esp_main->add_element(new c_checkbox(str("Customize color"), &vars.visuals.customize_color,
				[]() { return vars.visuals.nightmode; }));

			misc_esp_main->add_element(new c_slider(str("Nightmode amount"), &vars.visuals.nightmode_amount, 0, 100,
				[]() { return vars.visuals.nightmode && !vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_slider(str("Prop alpha amount"), &vars.visuals.prop_alpha_amount, 0, 100,
				[]() { return !vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_color, false,
				[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_text(str("World"), 0, []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_prop_color, true,
				[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_text(str("Props"), 0, []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_skybox_color, false,
				[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

			misc_esp_main->add_element(new c_text(str("Skybox"), 0, []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));
			break;
		case 9:
			misc_esp_main->add_element(new c_text(str("IMPACTS"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_checkbox(str("Show impacts"),
				&vars.visuals.bullet_impact));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.bullet_impact_color, true,
				[]() { return vars.visuals.bullet_impact; }));

			misc_esp_main->add_element(new c_text(str("Server impact"), 0, []() { return vars.visuals.bullet_impact; }));

			misc_esp_main->add_element(new c_colorpicker(&vars.visuals.client_impact_color, true,
				[]() { return vars.visuals.bullet_impact; }));

			misc_esp_main->add_element(new c_text(str("Client impact"), 0, []() { return vars.visuals.bullet_impact; }));

			misc_esp_main->add_element(new c_slider(str("Impacts size"), &vars.visuals.impacts_size, 0.f, 50.f));
			break;
		}

		misc_esp_main->initialize_elements();
	}
	void init() {
		groupboxes = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		groupboxes->set_size(Vector2D(192, 552));
		groupboxes->set_position(Vector2D(8, 48));

		groupboxes->set_padding(Vector2D(8, 12));

		auto subtab_selector = new c_listbox("", &current_subtab, {}, 0, true, nullptr, [](int) {
			misc_esp_main->update_state(init_misc_esp_main);
			});
		c_listbox::c_selector s1;
		s1.name = str("ENEMY");
		s1.elements = {
			str("ESP"), // 0
			str("Chams"), // 1
			str("Glow"), // 2
		};

		c_listbox::c_selector s2;
		s2.name = str("LOCAL");
		s2.elements = {
			str("Chams"), // 3
			str("Indicators"),// 4
			str("Effects"),// 5
			str("Other"),// 6
		};

		c_listbox::c_selector s3;
		s3.name = str("WORLD");
		s3.elements = {
			str("ESP"),// 7
			str("Effects"), // 8
			str("Other"),// 9
		};

		subtab_selector->make_subtabs();
		subtab_selector->add_selector(s1);
		subtab_selector->add_selector(s2);
		subtab_selector->add_selector(s3);

		groupboxes->add_element(subtab_selector);

		groupboxes->initialize_elements();
		g_Menu->window->add_element(groupboxes);
		misc_esp_main = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		misc_esp_main->set_size(Vector2D(376, 552));
		misc_esp_main->set_position(Vector2D(208, 48));
		misc_esp_main->set_padding(Vector2D(40, 8));
		init_misc_esp_main();
		g_Menu->window->add_element(misc_esp_main);
		g_Menu->window->set_title(str("Visuals"));
	}
}
*/









/*
namespace misc_tab {
	c_child* groupboxes = nullptr;
	c_child* misc_esp_main = nullptr;
	int current_subtab = 0;
	void init_misc_esp_main() {
		switch (current_subtab)
		{
		case 0:
			misc_esp_main->add_element(new c_text(str("MOVEMENT"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Auto jump"),
				&vars.misc.bunnyhop));

			misc_esp_main->add_element(new c_checkbox(str("Auto strafe"),
				&vars.misc.autostrafe));

			misc_esp_main->add_element(new c_checkbox(str("Slide walk"),
				&vars.misc.slidewalk));

			misc_esp_main->add_element(new c_keybind(str("Peek assistance"), &g_Binds[bind_peek_assist]));

			break;
		case 1:
			misc_esp_main->add_element(new c_text(str("BUY-BOT"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_checkbox(str("Buy-bot"),
				&vars.misc.autobuy.enable));

			misc_esp_main->add_element(new c_combo(str("Primary weapon"),
				&vars.misc.autobuy.main, {
					str("None"),
					str("Auto sniper"),
					str("Scout"),
					str("Awp"),
					str("Negev"),
					str("M249"),
					str("Rifle"),
					str("AUG/SG553"),
				}, []() { return vars.misc.autobuy.enable; }));

			misc_esp_main->add_element(new c_combo(str("Pistol"),
				&vars.misc.autobuy.pistol, {
					str("None"),
					str("Dual berettas"),
					str("P250"),
					str("CZ75-Auto"),
					str("Deagle/Revolver")
				}, []() { return vars.misc.autobuy.enable; }));

			misc_esp_main->add_element(new c_multicombo(str("Extra"),
				&vars.misc.autobuy.misc, {
					str("Head helmet"), // 1
					str("Other helmet"), // 2
					str("HE grenade"), // 4
					str("Molotov"), // 8
					str("Smoke"), // 16
					str("Taser"), // 32
					str("Defuse kit"), // 64
				}, []() { return vars.misc.autobuy.enable; }));
			break;
		case 2:
			misc_esp_main->add_element(new c_text(str("STYLE"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Debug mode"), &style.debug_mode));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::accented_color)));
			misc_esp_main->add_element(new c_text(str("Accented color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::window_background)));
			misc_esp_main->add_element(new c_text(str("Window background")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::window_background_hovered)));
			misc_esp_main->add_element(new c_text(str("Window background hovered")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::child_background)));
			misc_esp_main->add_element(new c_text(str("Child background")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::child_background_hovered)));
			misc_esp_main->add_element(new c_text(str("Child background hovered")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::button_color)));
			misc_esp_main->add_element(new c_text(str("Button color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::button_hovered_color)));
			misc_esp_main->add_element(new c_text(str("Button hovered color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::button_inactive_color)));
			misc_esp_main->add_element(new c_text(str("Button inactive color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::button_holding_color)));
			misc_esp_main->add_element(new c_text(str("Button holding color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::borders_color)));
			misc_esp_main->add_element(new c_text(str("Borders color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::borders_color_hovered)));
			misc_esp_main->add_element(new c_text(str("Borders color hovered")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::text_color)));
			misc_esp_main->add_element(new c_text(str("Text color")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::text_color_hovered)));
			misc_esp_main->add_element(new c_text(str("Text color hovered")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::text_color_active)));
			misc_esp_main->add_element(new c_text(str("Text color active")));

			misc_esp_main->add_element(new c_colorpicker(&style.get_color(c_style::text_color_inactive)));
			misc_esp_main->add_element(new c_text(str("Text color inactive")));
			break;
		case 3:
			misc_esp_main->add_element(new c_text(str("OTHER"), 25, nullptr, color_t(127, 127, 127)));

			misc_esp_main->add_element(new c_checkbox(str("Ragdoll force"),
				&vars.visuals.ragdoll_force));

			misc_esp_main->add_element(new c_checkbox(str("Clantag changer"),
				&vars.visuals.clantagspammer));

			misc_esp_main->add_element(new c_checkbox(str("Knife-bot"),
				&vars.misc.knifebot));

			misc_esp_main->add_element(new c_combo(str("Cheat mode"), &vars.misc.restrict_type, {
				str("Matchmaking"),
				str("HvH")
			}, nullptr));

			misc_esp_main->add_element(new c_checkbox(str("Anti-untrusted"),
				&vars.misc.antiuntrusted, []() {
					return vars.misc.restrict_type == 1;
				}));


			misc_esp_main->add_element(new c_button(str("Unload"), Vector2D(320, 32), []() { csgo->DoUnload = true; }));
			misc_esp_main->add_element(new c_button(str("Full update"), Vector2D(320, 32), []() { csgo->client_state->ForceFullUpdate(); }));

			misc_esp_main->add_element(new c_button(str("Spoof sv_cheats"), Vector2D(320, 32), []() {
				ConVar* sv_cheats = interfaces.cvars->FindVar(hs::sv_cheats.s().c_str());
				*(int*)((DWORD)&sv_cheats->m_fnChangeCallbacks + 0xC) = 0;
				sv_cheats->SetValue(1);
				}, []() { return vars.misc.restrict_type == 1; }));

			misc_esp_main->add_element(new c_button(str("Unlock developer's convars"), Vector2D(320, 32), EnableHiddenCVars, []() { return vars.misc.restrict_type == 1; }));

			break;
		}

		misc_esp_main->initialize_elements();
	}
	void init() {

		groupboxes = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		groupboxes->set_size(Vector2D(192, 552));
		groupboxes->set_position(Vector2D(8, 48));
		groupboxes->set_padding(Vector2D(8, 12));

		auto subtab_selector = new c_listbox("", &current_subtab, {}, 0, true, nullptr, [](int) {
			misc_esp_main->update_state(init_misc_esp_main);
			});

		c_listbox::c_selector s1;
		s1.name = str("MISC");
		s1.elements = {
			str("Movement"),
			str("Buy-bot"),
			str("Menu style"),
			str("Other"),
		};

		subtab_selector->make_subtabs();
		subtab_selector->add_selector(s1);

		groupboxes->add_element(subtab_selector);

		groupboxes->initialize_elements();
		g_Menu->window->add_element(groupboxes);
		misc_esp_main = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		misc_esp_main->set_size(Vector2D(376, 552));
		misc_esp_main->set_position(Vector2D(208, 48));
		misc_esp_main->set_padding(Vector2D(40, 8));
		init_misc_esp_main();
		g_Menu->window->add_element(misc_esp_main);
		g_Menu->window->set_title(str("Misc"));
	}
}

namespace skins_tab {
	c_child* groupboxes = nullptr;
	c_child* misc_esp_main = nullptr;
	int current_subtab = 0;
	void init_misc_esp_main() {
		static const std::vector<std::string> agent_skins = {
			str("Default"),
			str("Special Agent Ava"),
			str("Operator"),
			str("Markus Delrow"),
			str("Michael Syfers"),
			str("B Squadron Officer"),
			str("Seal Team 6 Soldier"),
			str("Buckshot"),
			str("Lt. Commander Ricksaw"),
			str("Third Commando Company"),
			str("'Two Times' McCoy"),
			str("Dragomir"),
			str("Rezan The Ready"),
			str("'The Doctor' Romanov"),
			str("Maximus"),
			str("Blackwolf"),
			str("The Elite Mr. Muhlik"),
			str("Ground Rebel"),
			str("Osiris"),
			str("Prof. Shahmat"),
			str("Enforcer"),
			str("Slingshot"),
			str("Soldier"),
			str("Street Soldier"),
			str("'Blueberries' Buckshot"),
			str("'Two Times' McCoy"),
			str("Rezan the Redshirt"),
			str("Dragomir"),
			str("Cmdr. Mae 'Dead Cold' Jamison"),
			str("001st Lieutenant Farlow"),
			str("John 'Van Healen' Kask"),
			str("Bio-Haz Specialist"),
			str("Sergeant Bombson"),
			str("Chem-Haz Specialist"),
			str("Sir Bloody Miami Darryl"),
			str("Sir Bloody Silent Darryl"),
			str("Sir Bloody Skullhead Darryl"),
			str("Sir Bloody Darryl Royale"),
			str("Sir Bloody Loudmouth Darryl"),
			str("Safecracker Voltzmanns"),
			str("Little Kev"),
			str("Number K"),
			str("Getaway Sally"),

		};
		switch (current_subtab)
		{
		case 0: {
			misc_esp_main->add_element(new c_text(str("AGENTS T"), 25, nullptr, color_t(127, 127, 127)));
			auto agents_t = new c_listbox("", &vars.misc.agents_t, agent_skins, 476, false);
			agents_t->set_width(296);
			misc_esp_main->add_element(agents_t);
		}
			break;
		case 1: {
			misc_esp_main->add_element(new c_text(str("AGENTS CT"), 25, nullptr, color_t(127, 127, 127)));
			auto agents_t = new c_listbox("", &vars.misc.agents_ct, agent_skins, 476, false);
			agents_t->set_width(296);
			misc_esp_main->add_element(agents_t);
		}
			  break;
		case 2:
			misc_esp_main->add_element(new c_text(str("SKINCHANGER"), 25, nullptr, color_t(127, 127, 127)));
			misc_esp_main->add_element(new c_text(str("Coming soon...")));
			break;
		}

		misc_esp_main->initialize_elements();
	}
	void init() {

		groupboxes = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		groupboxes->set_size(Vector2D(192, 552));
		groupboxes->set_position(Vector2D(8, 48));
		groupboxes->set_padding(Vector2D(8, 12));

		auto subtab_selector = new c_listbox("", &current_subtab, {}, 0, true, nullptr, [](int) {
			misc_esp_main->update_state(init_misc_esp_main);
			});

		c_listbox::c_selector s1;
		s1.name = str("MISC");
		s1.elements = {
			str("Models T"),
			str("Models CT"),
			str("Skinchanger"),
		};

		subtab_selector->make_subtabs();
		subtab_selector->add_selector(s1);

		groupboxes->add_element(subtab_selector);

		groupboxes->initialize_elements();
		g_Menu->window->add_element(groupboxes);
		misc_esp_main = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		misc_esp_main->set_size(Vector2D(376, 552));
		misc_esp_main->set_position(Vector2D(208, 48));
		misc_esp_main->set_padding(Vector2D(40, 8));
		init_misc_esp_main();
		g_Menu->window->add_element(misc_esp_main);
		g_Menu->window->set_title(str("Skins"));
	}
}

namespace configs_tab {
	c_child* cfg_list = nullptr;
	c_child* cfg_info = nullptr;
	c_child* cfg_actions = nullptr;
	
	void cfg_info_init() {
		if (ConfigList.empty())
			return;

		// PreLoad
		CConfig::preload_cfg p = CConfig::preload_cfg();
		Config.PreLoad(ConfigList[vars.menu.active_config_index], &p);
		if (p.can_be_loaded) {
			cfg_info->add_element(new c_text(ConfigList[vars.menu.active_config_index], 25, nullptr, color_t(127, 127, 127)));

			cfg_info->add_element(new c_text(str("Created at ") + p.created_at));
			cfg_info->add_element(new c_text(str("by ") + p.created_by));

			cfg_info->add_element(new c_text(str("Last modified at ") + p.last_modified_date));
			cfg_info->add_element(new c_text(str("by ") + p.last_modified_user));
		}
		else
		{
			cfg_info->add_element(new c_text(str("Config cannot be loaded.")));
		}
		cfg_info->initialize_elements();
	}

	void cfg_list_init() {
		cfg_list->add_element(new c_input_text(str("Name"), &vars.menu.active_config_name, false));

		cfg_list->add_element(new c_button(str("Create"), Vector2D(176, 32), []() {
			string add;
			if (vars.menu.active_config_name.find(str(".cfg")) == -1)
				add = str(".cfg");
			Config.Save(vars.menu.active_config_name + add, true);
			RefreshConfigs();
			cfg_list->update_state(cfg_list_init);
			cfg_info->update_state(cfg_info_init);
			vars.menu.active_config_name.clear();
		}, []() { return vars.menu.active_config_name.size() > 0; }));


		auto subtab_selector = new c_listbox(str("CONFIGS LIST"), &vars.menu.active_config_index, ConfigList, 0, true, nullptr, [](int) {
			cfg_info->update_state(cfg_info_init);
			});

		cfg_list->add_element(subtab_selector);
		cfg_list->initialize_elements();
	}

	void init() {
		RefreshConfigs();

		cfg_list = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		cfg_list->set_position(Vector2D(8, 48));
		cfg_list->set_size(Vector2D(192, 552));
		cfg_list->set_padding(Vector2D(8, 12));

		cfg_info = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		cfg_info->set_size(Vector2D(376, 224));
		cfg_info->set_position(Vector2D(208, 48));
		cfg_info->set_padding(Vector2D(40, 8));

		cfg_actions = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		cfg_actions->set_size(Vector2D(376, 320));
		cfg_actions->set_position(Vector2D(208, 280));
		cfg_actions->set_padding(Vector2D(8, 8));

		cfg_actions->add_element(new c_button(str("Load"), Vector2D(360, 32), []() {
			Config.Load(ConfigList[vars.menu.active_config_index]);
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		cfg_actions->add_element(new c_button(str("Save"), Vector2D(360, 32), []() {
			Config.Save(ConfigList[vars.menu.active_config_index]);
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		cfg_actions->add_element(new c_button(str("Reset"), Vector2D(360, 32), []() { Config.ResetToDefault(); },
			[]() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		cfg_actions->add_element(new c_button(str("Delete"), Vector2D(360, 32)));
		cfg_actions->add_element(new c_button(str("Refresh"), Vector2D(360, 32), []() {
			RefreshConfigs();
			cfg_list->update_state(cfg_list_init);
			cfg_info->update_state(cfg_info_init);
			}));

		cfg_actions->add_element(new c_button(str("Reset to default"), Vector2D(360, 32), []() { Config.ResetToDefault(); },
			[]() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		cfg_actions->add_element(new c_button(str("Open folder"), Vector2D(360, 32), []() {
			static TCHAR path[MAX_PATH];
			std::string folder;

			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
			{
				folder = std::string(path) + str("\\weave\\configs\\");
			}
			ShellExecute(NULL, NULL, folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}));

		cfg_actions->initialize_elements();

		cfg_info_init();
		cfg_list_init();
		g_Menu->window->add_element(cfg_list);
		g_Menu->window->add_element(cfg_info);
		g_Menu->window->add_element(cfg_actions);
		g_Menu->window->set_title(str("Configs"));
	}
}

namespace javascipt_tab {
	c_child* scripts_list = nullptr;
	c_child* scripts_info = nullptr;
	c_child* scripts_actions = nullptr;
	int current_script = 0;

	void scripts_actions_init();

	void scripts_info_init() {
		scripts_info->add_element(new c_text(str("SCRIPT INFO"), 25, nullptr, color_t(127, 127, 127)));
		if (js_loader.scripts_list.empty())
			return;
		scripts_info->add_element(new c_text(js_loader.scripts_list[current_script]));
		
		for (auto el : js_handler.menu_elements[js_loader.scripts_list[current_script]])
		{
			switch (el.second->p_element->type)
			{
			case c_elementtype::checkbox:
				scripts_info->add_element(((c_checkbox*)el.second->p_element)->clone());
				break;
			case c_elementtype::slider:
				scripts_info->add_element(((c_slider*)el.second->p_element)->clone());
				break;
			}
		}
		scripts_info->initialize_elements();
	}
	
	void scripts_list_init() {
		if (js_loader.scripts_list.empty())
			return;

		auto subtab_selector = new c_listbox(str("SCRIPTS LIST"), &current_script, js_loader.scripts_list, 0, true, nullptr, [](int) {
			scripts_info->update_state(scripts_info_init);
			scripts_actions->update_state(scripts_actions_init);
		});
	
		scripts_list->add_element(subtab_selector);
		scripts_list->initialize_elements();
	}
	void scripts_actions_init() {
		scripts_actions->add_element(new c_text(str("GENERAL"), -25, nullptr, color_t(127, 127, 127)));
		if (!js_loader.scripts_list.empty())
		{
			if (js_loader.loaded_scripts[js_loader.scripts_list[current_script]]) {
				scripts_actions->add_element(new c_button(str("Unload"), Vector2D(360, 32), []() {
					js_loader.unload(js_loader.scripts_list[current_script]);
					scripts_actions->update_state(scripts_actions_init);
					scripts_info->update_state(scripts_info_init);
					}, []() { return js_loader.scripts_list.size() > 0 && current_script >= 0; }));
			}
			else {
				scripts_actions->add_element(new c_button(str("Load"), Vector2D(360, 32), []() {
					js_loader.load(js_loader.scripts_list[current_script]);
					scripts_actions->update_state(scripts_actions_init);
					scripts_info->update_state(scripts_info_init);
					}, []() { return js_loader.scripts_list.size() > 0 && current_script >= 0; }));
			}
		}

		scripts_actions->add_element(new c_button(str("Refresh scripts"), Vector2D(360, 32), []() {
			js_loader.refresh_scripts();
			scripts_list->update_state(scripts_list_init);
			scripts_info->update_state(scripts_info_init);
			}));
		scripts_actions->add_element(new c_button(str("Open scripts folder"), Vector2D(360, 32), []() {
			static TCHAR path[MAX_PATH];
			std::string folder;

			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
			{
				folder = std::string(path) + str("\\weave\\scripts\\");
			}
			CreateDirectory(folder.c_str(), NULL);

			ShellExecute(NULL, NULL, folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}));
		scripts_actions->initialize_elements();
	}

	void init() {
		js_loader.refresh_scripts();

		scripts_list = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		scripts_list->set_position(Vector2D(8, 48));
		scripts_list->set_size(Vector2D(192, 552));
		scripts_list->set_padding(Vector2D(8, 12));

		scripts_info = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		scripts_info->set_size(Vector2D(376, 224));
		scripts_info->set_position(Vector2D(208, 48));
		scripts_info->set_padding(Vector2D(40, 8));

		scripts_actions = new c_child("", -1, g_Menu->window, c_child::child_alignment::none);
		scripts_actions->set_size(Vector2D(376, 320));
		scripts_actions->set_position(Vector2D(208, 280));
		scripts_actions->set_padding(Vector2D(8, 8));

		scripts_info_init();
		scripts_list_init();
		scripts_actions_init();

		g_Menu->window->add_element(scripts_list);
		g_Menu->window->add_element(scripts_info);
		g_Menu->window->add_element(scripts_actions);
		g_Menu->window->set_title(str("Scripts"));
	}
}

*/
void tab_init(void(*fn)());

#define create_texture(tex_name, bytes) static LPDIRECT3DTEXTURE9 tex_name; \
										if (tex_name == NULL) \
											D3DXCreateTextureFromFileInMemory(g_Render->GetDevice(), &bytes, sizeof(bytes), &tex_name)
/*
void init_authorization_form()
{
	create_texture(texture_configs, menu_icon_configs);
	create_texture(texture_legit, menu_icon_legit);
	create_texture(texture_misc, menu_icon_misc);
	create_texture(texture_profile, menu_icon_profile);
	create_texture(texture_rage, menu_icon_rage);
	create_texture(texture_skins, menu_icon_skins);
	create_texture(texture_visuals, menu_icon_visuals);

	auto child = new c_child(str(""), -1, g_Menu->window, c_child::child_alignment::none);
	{
		child->set_size(Vector2D(576 + 16, 600));
		child->set_padding(Vector2D(8, 48));
		child->set_visible(false);
		child->set_elements_padding_y(3);

		child->add_element(new c_tab_selector(str("RAGEBOT"), str("Get maximum superiority over the enemy!"), texture_rage,
			[]() {
				tab_init(ragebot_tab::init);
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("LEGITBOT"), str("The most invisible help to become more powerful!"), texture_legit,
			[]() {
				
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("VISUALS"), str("Get an aesthetic and beautiful advantage over the enemy!"), texture_visuals,
			[]() {
				tab_init(visuals_tab::init);
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("MISC"), str("Miscellaneous misc_esp_main."), texture_misc,
			[]() {
				tab_init(misc_tab::init);
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("SKINS"), str("Setup your awesome skins!"), texture_skins,
			[]() {
				tab_init(skins_tab::init);
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("CONFIG"), str("Customize your configuration and share with friends!"), texture_configs,
			[]() {
				tab_init(configs_tab::init);
			},
			[]() {
				return true;
			}));

		child->add_element(new c_tab_selector(str("SCRIPTS"), str("Extend your cheat experience with own scripts!"), texture_profile,
			[]() {
				tab_init(javascipt_tab::init);
			},
			[]() {
				return true;
			}));
	}
	child->initialize_elements();
	g_Menu->window->add_element(child);

	g_Menu->window->set_on_click_back(nullptr);
	g_Menu->window->set_title(str("Main"));
}

void tab_init(void(*fn)()) {

	g_Menu->window->set_on_click_back([]() {
		g_Menu->window->update_state(init_authorization_form);
	});

	g_Menu->window->update_state(fn);
}
*/
/*
void CMenu::render() {
	if (!ImGui::GetCurrentContext())
		return;

	auto flags_backup = g_Render->_drawList->Flags;
	g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	static bool once = false;
	if (!once) {
		Config.ResetToDefault();
		vars.menu.open = true;
		once = true;
	}

	static float alpha = 0.f;

	if (initialized) {
		window->update_keystates();
		window->update_animation();
		update_binds();
		if (vars.menu.open)
			alpha += animation_speed / 1.5f;
		else
			alpha -= animation_speed / 1.5f;
		alpha = clamp(alpha, 0.f, 1.f);
		window->set_transparency(sin(DEG2RAD(alpha * 90.f)) * 100.f);
	}
	else {
		g_Menu->window = new c_window();
		g_Menu->window->set_active_tab_index(-1);
		g_Menu->window->set_position({ 0, 0 });
		static const Vector2D& wnd_size = Vector2D{ 592, 608 };
		window->set_size(wnd_size);
		window->set_position(Vector2D(csgo->w / 2 - wnd_size.x / 2, csgo->h / 2 - wnd_size.y / 2));

		init_authorization_form();
		style.init();
		initialized = true;
	}

	ImGui::GetIO().MouseDrawCursor = window->get_transparency() > 0.f;
	POINT mp;
	GetCursorPos(&mp);
	ScreenToClient(GetForegroundWindow(), &mp);
	g_mouse.x = mp.x;
	g_mouse.y = mp.y;

	window->render();
	if (window->g_hovered_element) {
		if (window->g_hovered_element->type == c_elementtype::input_text)
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
	}
	csgo->scroll_amount = 0;

	g_Render->_drawList->Flags = flags_backup;
}

*/



void CMenu::render() {
	if (!ImGui::GetCurrentContext())
		return;

	auto flags_backup = g_Render->_drawList->Flags;
	g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	static bool once = false;
	if (!once) {
		Config.ResetToDefault();
		vars.menu.open = true;
		once = true;
	}

	static float alpha = 0.f;

	if (initialized) {
		window->update_keystates();
		window->update_animation();
		update_binds();
		if (vars.menu.open)
			alpha += animation_speed / 1.5f;
		else
			alpha -= animation_speed / 1.5f;
		alpha = clamp(alpha, 0.f, 1.f);
		window->set_transparency(sin(DEG2RAD(alpha * 90.f)) * 100.f);
		if (window->get_transparency() < 100.f && vars.menu.open)
			window->increase_transparency(animation_speed * 80.f);
	}
	else {
		window = new c_window();
		window->set_size(Vector2D(670 , 600));
		static int SCREENone, SCREENtwo;
		interfaces.engine->GetScreenSize(SCREENone, SCREENtwo);
		window->set_position(Vector2D((SCREENone - 900) * .5f, (SCREENtwo - 600) * .5f));
		window->add_tab(new c_tab("A", tab_t::rage, window)); {

			auto main_child = new c_child("Aimbot", tab_t::rage, window);
			main_child->set_size(Vector2D(255, 550));
			main_child->set_position(Vector2D(92, -40)); {
				
				main_child->add_element(new c_checkbox(str("Enable"), &vars.ragebot.enable));
				main_child->add_element(new c_slider(str("FOV"), &vars.ragebot.fov, 0.f, 180.f, "%.0f", enable_rage));

				main_child->add_element(new c_checkbox(str("Silent aim"),
					&vars.ragebot.silent, enable_rage));

				main_child->add_element(new c_checkbox(str("Auto fire"),
					&vars.ragebot.autoshoot, enable_rage));

				main_child->add_element(new c_combo(str("Auto scope"), &vars.ragebot.autoscope, {
					str("Off"), str("Always"), str("Hitchance fail") }, enable_rage));
				main_child->add_element(new c_checkbox(str("Auto stop"), &vars.ragebot.weapon[vars.ragebot.active_index].quickstop,
					[]() { return enable_rage(); }));


				//main_child->add_element(new c_keybind(str("Override damage"),



			

	
				main_child->add_element(new c_multicombo(str("Options"), &vars.ragebot.weapon[vars.ragebot.active_index].quickstop_options, {
						str("Between shots"),
						str("Force accuracy")
					},
					[]() {
						return enable_rage() && vars.ragebot.weapon[vars.ragebot.active_index].quickstop;
					}));
				main_child->add_element(new c_keybind(str("Double-tap"), &g_Binds[bind_double_tap], []() {
					return vars.ragebot.enable;
					}));
				main_child->add_element(new c_slider(str("Doubletap HC"), &vars.ragebot.weapon[vars.ragebot.active_index].doubletap_hc, 0, 100, "%.0f",
					[]() {
						return enable_rage()
							&& (g_Binds[bind_double_tap].key > 0 || g_Binds[bind_double_tap].active)
							&& vars.ragebot.active_index != 2
							&& vars.ragebot.active_index != 3;
					}));
				main_child->add_element(new c_slider(str("Charge delay"), &vars.ragebot.recharge_time,
					0, 14, "%.0f", enable_rage));

				main_child->add_element(new c_checkbox(str("Teleport boost"), &vars.ragebot.dt_teleport, []() {
					return enable_rage() && (g_Binds[bind_double_tap].key > 0 || g_Binds[bind_double_tap].active);
					}));

				main_child->add_element(new c_keybind(str("Hide-shots"), &g_Binds[bind_hide_shots], []() {
					return vars.ragebot.enable;
					}));


				main_child->add_element(new c_keybind(str("Force safe points"), &g_Binds[bind_force_safepoint], enable_rage));
				main_child->add_element(new c_keybind(str("Force body aim"), &g_Binds[bind_baim], enable_rage));



				main_child->initialize_elements();
			}
			window->add_element(main_child);
			auto sec_child = new c_child("Other", tab_t::rage, window);
			sec_child->set_size(Vector2D(255, 550));
			sec_child->set_position(Vector2D(372, -40)); {
				sec_child->add_element(new c_checkbox(str("Resolver"), &vars.ragebot.resolver,
					enable_rage));




		
				sec_child->add_element(new c_multicombo("hitboxes", &vars.ragebot.weapon[vars.ragebot.active_index].hitscan, {
	"head",
	"neck",
	"upper chest",
	"chest",
	"stomach",
	"pelvis",
	"arms",
	"legs",
	"feet",
					}, override_default));
				sec_child->add_element(new c_slider("hitchance", &vars.ragebot.weapon[vars.ragebot.active_index].hitchance, 0, 100, "%.0f%%",
					override_default));

				sec_child->add_element(new c_slider("minimum damage", &vars.ragebot.weapon[vars.ragebot.active_index].mindamage, 0, 120, "%.0f hp",
					override_default));
				sec_child->add_element(new c_checkbox("multipoint", &vars.ragebot.weapon[vars.ragebot.active_index].multipoint, override_default));

				sec_child->add_element(new c_slider("head scale", &vars.ragebot.weapon[vars.ragebot.active_index].scale_head,
					0, 100, "%.0f%%", []() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].multipoint; }));

				sec_child->add_element(new c_slider("body scale", &vars.ragebot.weapon[vars.ragebot.active_index].scale_body,
					0, 100, "%.0f%%", []() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].multipoint; }));

	



				sec_child->add_element(new c_slider("max misses", &vars.ragebot.weapon[vars.ragebot.active_index].max_misses,
					0, 5, "%.0f", []() {
						return override_default()
							&& (vars.ragebot.weapon[vars.ragebot.active_index].max_misses & 2);
					}));



				sec_child->initialize_elements();
			}
			window->add_element(sec_child);

			
		}

		window->set_transparency(100.f);
		ImGui::Spacing();
		
		window->add_tab(new c_tab("G", tab_t::antiaims, window)); {
			auto aa_child = new c_child("AntiAim", tab_t::antiaims, window);
			aa_child->set_size(Vector2D(255, 550));
			aa_child->set_position(Vector2D(92, -40)); {
				aa_child->add_element(new c_checkbox("enable", &vars.antiaim.enable));

				aa_child->add_element(new c_checkbox(str("On use"), &vars.antiaim.aa_on_use,
					[]() { return enable_antiaim(); }));

				aa_child->add_element(new c_checkbox(str("At target"),
					&vars.antiaim.attarget, []() { return enable_antiaim(); }));

				aa_child->add_element(new c_checkbox(str("Off when offscreen"),
					&vars.antiaim.attarget_off_when_offsreen, []() { return enable_antiaim() && vars.antiaim.attarget; }));

				aa_child->add_element(new c_checkbox(str("Manual yaw direction"),
					&vars.antiaim.manual_antiaim, enable_antiaim));

				
				aa_child->add_element(new c_keybind(str("Inverter"), &g_Binds[bind_aa_inverter],
					[]() { return enable_antiaim() && vars.antiaim.desync_direction == 0 && vars.antiaim.desync; }));
				aa_child->add_element(new c_checkbox(str("Ignore at target"),
					&vars.antiaim.ignore_attarget, []() { return enable_antiaim() && vars.antiaim.manual_antiaim; }));

				aa_child->add_element(new c_keybind(str("Left"), &g_Binds[bind_manual_left], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				aa_child->add_element(new c_keybind(str("Right"), &g_Binds[bind_manual_right], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				aa_child->add_element(new c_keybind(str("Back"), &g_Binds[bind_manual_back], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				aa_child->add_element(new c_keybind(str("Forward"), &g_Binds[bind_manual_forward], []() {
					return enable_antiaim() && vars.antiaim.manual_antiaim;
					}));

				aa_child->add_element(new c_slider(str("desync"), &vars.antiaim.desync_amount, 0, 180, "%.0f%%",
					[]() { return enable_antiaim() && vars.antiaim.desync; }));
				aa_child->add_element(new c_slider(str("yaw Amount"), &vars.antiaim.yaw_amount, 0, 180, "%.0f%%",
					[]() { return enable_antiaim() && vars.antiaim.desync; }));
				aa_child->add_element(new c_slider(str("Jitter angle"), &vars.antiaim.jitter_angle, 0.f, 45.f, "%.0f%%",
					[]() { return enable_antiaim(); }));
				aa_child->add_element(new c_combo(str("Pitch direction"), &vars.antiaim.pitch, {
					str("Disabled"),
					str("Down"),
					str("Ideal down"),
					}, []() { return enable_antiaim(); }));

				aa_child->add_element(new c_combo(str("Yaw direction"), &vars.antiaim.yaw, {
					str("Disabled"),
					str("Backwards"),
					}, []() { return enable_antiaim(); }));

				aa_child->add_element(new c_checkbox(str("Enable"), &vars.antiaim.desync,
					[]() { return enable_antiaim(); }));
				aa_child->add_element(new c_combo(str("Type"), &vars.antiaim.desync_direction,
					{
						str("Manual"),
						str("Peek real"),
						str("Peek desync"),
						str("Jitter")
					},
					[]() { return enable_antiaim() && vars.antiaim.desync; }));

				aa_child->initialize_elements();


			}
			window->add_element(aa_child);

			auto fakelag_main = new c_child("fakelag", tab_t::antiaims, window);
			fakelag_main->set_size(Vector2D(255, 260));
			fakelag_main->set_position(Vector2D(372, -40)); {

				fakelag_main->add_element(new c_combo(str("Lag mode"), &vars.antiaim.fakelag,
					{
						str("Disabled"),
						str("Static"),
						str("Fluctuate"),
						str("Adaptive"),
					}));

				

				fakelag_main->add_element(new c_slider(str("Limit"), &vars.antiaim.fakelagfactor, 1, 62, "%.0f%%",
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				

				fakelag_main->add_element(new c_checkbox(str("When standing"), &vars.antiaim.fakelag_when_standing,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				fakelag_main->add_element(new c_checkbox(str("When using exploits"), &vars.antiaim.fakelag_when_exploits,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));

				fakelag_main->add_element(new c_checkbox(str("Only on peek"), &vars.antiaim.fakelag_on_peek,
					[]() { return enable_antiaim() && vars.antiaim.fakelag > 0; }));
				fakelag_main->initialize_elements();
			}
			window->add_element(fakelag_main);
			auto other_main = new c_child("Other", tab_t::antiaims, window);
			other_main->set_size(Vector2D(255, 260));
			other_main->set_position(Vector2D(372, 250)); {

				other_main->add_element(new c_keybind(str("Fake duck"), &g_Binds[bind_fake_duck],
					[]() { return enable_antiaim() && vars.misc.restrict_type == 1; }));

				other_main->add_element(new c_keybind(str("Slow walk"), &g_Binds[bind_slow_walk],
					[]() { return enable_antiaim(); }));
				other_main->initialize_elements();
			}
			window->add_element(other_main);
		}
		ImGui::Spacing();
		window->add_tab(new c_tab("B", tab_t::legit, window)); {
			
		}
		ImGui::Spacing();
		window->add_tab(new c_tab("C", tab_t::esp, window)); {
			
			auto esp_main = new c_child("Player ESP", tab_t::esp, window);
			esp_main->set_size(Vector2D(255, 300));
			esp_main->set_position(Vector2D(92, -40)); {

				esp_main->add_element(new c_checkbox(str("Enable"),
					&vars.visuals.enable));

				esp_main->add_element(new c_checkbox(str("Dormant"),
					&vars.visuals.dormant, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.box_color, false,
					[]() { return enable_esp() && vars.visuals.box; }));

				esp_main->add_element(new c_checkbox(str("Box"),
					&vars.visuals.box, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.name_color, true,
					[]() { return enable_esp() && vars.visuals.name; }));

				esp_main->add_element(new c_checkbox(str("Name"),
					&vars.visuals.name, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.weapon_color, true,
					[]() { return enable_esp() && vars.visuals.weapon; }));

				esp_main->add_element(new c_checkbox(str("Weapon"),
					&vars.visuals.weapon, enable_esp));

				esp_main->add_element(new c_checkbox(str("Health"),
					&vars.visuals.healthbar, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.hp_color, false,
					[]() { return enable_esp() && vars.visuals.healthbar && vars.visuals.override_hp; }));

				esp_main->add_element(new c_checkbox(str("Override health color"),
					&vars.visuals.override_hp, [] { return enable_esp() && vars.visuals.healthbar; }));

				esp_main->add_element(new c_colorpicker(&vars.visuals.ammo_color, true,
					[]() { return enable_esp() && vars.visuals.ammo; }));

				esp_main->add_element(new c_checkbox(str("Ammo"),
					&vars.visuals.ammo, enable_esp));

				esp_main->add_element(new c_checkbox(str("Zeus warning"),
					&vars.visuals.zeus_warning, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.out_of_fov_color, true,
					[]() { return enable_esp() && vars.visuals.out_of_fov; }));

				esp_main->add_element(new c_checkbox(str("Out of fov arrow"),
					&vars.visuals.out_of_fov, enable_esp));

				esp_main->add_element(new c_slider(str("Distance"), &vars.visuals.out_of_fov_distance, 0, 300,"%.0f%%",
					[]() { return enable_esp() && vars.visuals.out_of_fov; }));

				esp_main->add_element(new c_slider(str("Size"), &vars.visuals.out_of_fov_size, 10, 35,"%.0f%%",
					[]() { return enable_esp() && vars.visuals.out_of_fov; }));

				esp_main->add_element(new c_checkbox(str("Show multipoint"), &vars.visuals.shot_multipoint, enable_esp));

				esp_main->add_element(new c_multicombo(str("Flags"),
					&vars.visuals.flags, {
						str("Armor"),
						str("Scoped"),
						str("Flashed"),
						str("Defuse kit"),
						str("Fake duck"),
						str("Distance"),
						str("Position"),
					}, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.flags_color, true,
					[]() { return enable_esp() && vars.visuals.flags > 0; }));

				esp_main->add_element(new c_text("Flags color",  []() { return enable_esp() && vars.visuals.flags > 0; }));
				
				esp_main->initialize_elements();
			}
			window->add_element(esp_main);
			int current_subtab = 0;
			auto misc_esp_main = new c_child("Other ESP", tab_t::esp, window);
			misc_esp_main->set_size(Vector2D(255, 550));
			misc_esp_main->set_position(Vector2D(372, -40)); {


				misc_esp_main->add_element(new c_checkbox(str("Watermark"), &vars.visuals.watermark));
				misc_esp_main->add_element(new c_checkbox(str("Keybind states"), &vars.visuals.indicators));
				misc_esp_main->add_element(new c_checkbox(str("Indicators"), &vars.visuals.indicators));
				misc_esp_main->add_element(new c_checkbox(str("Ragebot only"), &vars.visuals.indicators_rage,
					[]() { return vars.visuals.indicators; }));

				misc_esp_main->add_element(new c_combo(str("Taser range"), &vars.visuals.taser_range, { str("Off"), str("Rainbow"), str("Default") }));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.taser_range_color, true,
					[]() { return vars.visuals.taser_range > 0; }));

				misc_esp_main->add_element(new c_text(str("Taser color"), 
					[]() { return vars.visuals.taser_range > 0; }));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nadepred_color, true,
					[]() { return vars.visuals.nadepred; }));

				misc_esp_main->add_element(new c_checkbox(str("Grenade prediction"),
					&vars.visuals.nadepred));
				misc_esp_main->add_element(new c_multicombo(str("Eventlog"), &vars.visuals.eventlog, {
					str("Damage"), str("Aimbot"), str("Missed shots"), str("Misc"),
					}));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.eventlog_color, false,
					[]() { return vars.visuals.eventlog > 0; }));

				misc_esp_main->add_element(new c_text(str("Eventlog color"),  []() { return vars.visuals.eventlog > 0; }));
				misc_esp_main->add_element(new c_checkbox(str("Kill effect"),
					&vars.visuals.kill_effect));

				misc_esp_main->add_element(new c_slider(str("Aspect ratio"), &vars.visuals.aspect_ratio, 0, 250, "%.0f"));

		
				misc_esp_main->add_element(new c_slider(str("Viewmodel fov"), &vars.misc.viewmodelfov, 68, 145, "%.0f"));

				misc_esp_main->add_element(new c_slider(str("Offset X"), &vars.misc.viewmodel_x, -25.f, 25.f, "%.0f" ));
				misc_esp_main->add_element(new c_slider(str("Offset Y"), &vars.misc.viewmodel_y, -25.f, 25.f, "%.0f"));
				misc_esp_main->add_element(new c_slider(str("Offset Z"), &vars.misc.viewmodel_z, -25.f, 25.f, "%.0f"));
				misc_esp_main->add_element(new c_keybind(str("Thirdperson"), &g_Binds[bind_third_person]));

				misc_esp_main->add_element(new c_slider(str("Distance"), &vars.visuals.thirdperson_dist, 0, 300,  "%.0f" ,[]() {
					return g_Binds[bind_third_person].key > 0;
					}));

				misc_esp_main->add_element(new c_checkbox(str("Save death notices"),
					&vars.visuals.preverse_killfeed));

				misc_esp_main->add_element(new c_checkbox(str("Force crosshair"),
					&vars.visuals.force_crosshair));

				misc_esp_main->add_element(new c_multicombo(str("Hitmarker"),
					&vars.visuals.hitmarker, { str("Screen center"), str("World") }));

				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.hitmarker_color, false, []() { return vars.visuals.hitmarker > 0; }));
				misc_esp_main->add_element(new c_text(str("Hitmarker color",  []() { return vars.visuals.hitmarker > 0; })));

				misc_esp_main->add_element(new c_checkbox(str("Show damage"),
					&vars.visuals.visualize_damage, []() {
						return vars.visuals.hitmarker > 0;
					}));

				misc_esp_main->add_element(new c_checkbox(str("Hit sound"),
					&vars.visuals.hitmarker_sound));

				misc_esp_main->add_element(new c_combo(str("Sound type"),
					&vars.visuals.hitmarker_sound_type, { str("Default"), str("Warning"), str("Click"), str("COD") }, []() {
						return vars.visuals.hitmarker_sound;
					}));
				misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.weapons.color, true,
					[]() { return vars.visuals.world.weapons.enabled; }));

				misc_esp_main->add_element(new c_checkbox(str("Show weapons"), &vars.visuals.world.weapons.enabled));
				misc_esp_main->add_element(new c_checkbox(str("Bomb timer"), &vars.visuals.world.weapons.planted_bomb));

				misc_esp_main->add_element(new c_checkbox(str("Show grenades"), &vars.visuals.world.projectiles.enable));

				
					static const auto& projectiles_enabled = []() {
						return vars.visuals.world.projectiles.enable;
					};
					misc_esp_main->add_element(new c_multicombo(str("Filter"), &vars.visuals.world.projectiles.filter,
						{
							str("Team"),
							str("Enemy"),
							str("Local player")
						}, projectiles_enabled));

					misc_esp_main->add_element(new c_checkbox(str("Trajectories"), &vars.visuals.world.projectiles.trajectories, projectiles_enabled));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[0], false,
						[]() { return vars.visuals.world.projectiles.enable
						&& vars.visuals.world.projectiles.filter & 1; }));

					misc_esp_main->add_element(new c_text(str("Team color"), 
						[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 1; }));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[1], false,
						[]() { return vars.visuals.world.projectiles.enable
						&& vars.visuals.world.projectiles.filter & 2; }));

					misc_esp_main->add_element(new c_text(str("Enemy color"), 
						[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 2; }));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.world.projectiles.colors[2], false,
						[]() { return vars.visuals.world.projectiles.enable
						&& vars.visuals.world.projectiles.filter & 4; }));

					misc_esp_main->add_element(new c_text(str("Local color"), 
						[]() { return vars.visuals.world.projectiles.enable && vars.visuals.world.projectiles.filter & 4; }));
					misc_esp_main->add_element(new c_multicombo(str("Removals"),
						&vars.visuals.remove, {
							str("Visual recoil"), // 1
							str("Smoke"), // 2
							str("Flash"), // 4
							str("Scope"), // 8
							str("Post processing"), // 16
							str("Fog"), // 32
							str("World shadow"), // 64
							str("Foot shadow"), // 128
							str("Viewmodel bob"), // 256
						}));
					misc_esp_main->add_element(new c_slider(str("World fov"), &vars.misc.worldfov, 0, 50, "%.0f" ));

					misc_esp_main->add_element(new c_slider(str("Zoom fov"), &vars.misc.zoomfov, 0, 100, "%.0f"));

					misc_esp_main->add_element(new c_checkbox(str("Night mode"), &vars.visuals.nightmode));

					misc_esp_main->add_element(new c_checkbox(str("Customize color"), &vars.visuals.customize_color,
						[]() { return vars.visuals.nightmode; }));

					misc_esp_main->add_element(new c_slider(str("Nightmode amount"), &vars.visuals.nightmode_amount, 0, 100, "%.0f",
						[]() { return vars.visuals.nightmode && !vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_slider(str("Prop alpha amount"), &vars.visuals.prop_alpha_amount, 0, 100, "%.0f",
						[]() { return !vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_color, false,
						[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_text(str("World"),  []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_prop_color, true,
						[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_text(str("Props"), []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_colorpicker(&vars.visuals.nightmode_skybox_color, false,
						[]() { return vars.visuals.nightmode && vars.visuals.customize_color; }));

					misc_esp_main->add_element(new c_text(str("Skybox"),  []() { return vars.visuals.nightmode && vars.visuals.customize_color; }));
	
						misc_esp_main->add_element(new c_checkbox(str("Show impacts"),
						&vars.visuals.bullet_impact));

						misc_esp_main->add_element(new c_colorpicker(&vars.visuals.bullet_impact_color, true,
						[]() { return vars.visuals.bullet_impact; }));

						misc_esp_main->add_element(new c_text(str("Server impact"),  []() { return vars.visuals.bullet_impact; }));

						misc_esp_main->add_element(new c_colorpicker(&vars.visuals.client_impact_color, true,
							[]() { return vars.visuals.bullet_impact; }));

						misc_esp_main->add_element(new c_text(str("Client impact"), []() { return vars.visuals.bullet_impact; }));

						misc_esp_main->add_element(new c_slider(str("Impacts size"), &vars.visuals.impacts_size, 0.f, 50.f, "%.0f"));
				misc_esp_main->initialize_elements();
			}
			window->add_element(misc_esp_main);
			auto other_esp_main = new c_child("Colored models", tab_t::esp, window);
			other_esp_main->set_size(Vector2D(255, 220));
			
			other_esp_main->set_position(Vector2D(92, 290)); {
				other_esp_main->add_element(new c_checkbox(str("Player "), &vars.visuals.chams[enemy_visible].enable)); {
					auto enemy_visible_enabled = []() { return vars.visuals.chams[enemy_visible].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_visible].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, enemy_visible_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].material_color, true, enemy_visible_enabled));
					other_esp_main->add_element(new c_text(str("Material color"), enemy_visible_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].glass_color, true, []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"), []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_visible].metallic_color, true, []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"), []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_visible].phong_amount, 0, 100, "%.0f", []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_visible].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[enemy_visible].enable && vars.visuals.chams[enemy_visible].material == 3;
							}));
					}
				}

				other_esp_main->add_element(new c_checkbox(str("Player behind wall"), &vars.visuals.chams[enemy_xqz].enable)); {
					auto enemy_xqz_enabled = []() { return vars.visuals.chams[enemy_xqz].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[enemy_xqz].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, enemy_xqz_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].material_color, true, enemy_xqz_enabled));
					other_esp_main->add_element(new c_text(str("Material color"), enemy_xqz_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].glass_color, true, []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"), []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[enemy_xqz].metallic_color, true, []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"), []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[enemy_xqz].phong_amount, 0, 100, "%.0f", []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[enemy_xqz].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[enemy_xqz].enable && vars.visuals.chams[enemy_xqz].material == 3;
							}));
					}
				}

				other_esp_main->add_element(new c_checkbox(str("Local player "), &vars.visuals.chams[local_default].enable)); {


					auto local_default_enabled = []() { return vars.visuals.chams[local_default].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_default].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, local_default_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].material_color, true, local_default_enabled));
					other_esp_main->add_element(new c_text(str("Material color"), local_default_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].glass_color, true, []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"), []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_default].metallic_color, true, []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"), []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_default].phong_amount, 0, 100, "%.0f", []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_default].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[local_default].enable && vars.visuals.chams[local_default].material == 3;
							}));
					}
				}
				
				other_esp_main->add_element(new c_checkbox(str("Local player fake"), &vars.visuals.chams[local_desync].enable));
				{
					auto local_desync_enabled = []() { return vars.visuals.chams[local_desync].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_desync].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, local_desync_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].material_color, true, local_desync_enabled));
					other_esp_main->add_element(new c_text(str("Material color"), local_desync_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].glass_color, true, []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"), []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_desync].metallic_color, true, []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"), []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_desync].phong_amount, 0, 100, "%.0f", []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_desync].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[local_desync].enable && vars.visuals.chams[local_desync].material == 3;
							}));
					}
				}
				other_esp_main->add_element(new c_checkbox(str("Hands"), &vars.visuals.chams[local_arms].enable));
				{
					auto local_arms_enabled = []() { return vars.visuals.chams[local_arms].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_arms].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, local_arms_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].material_color, true, local_arms_enabled));
					other_esp_main->add_element(new c_text(str("Material color"),  local_arms_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].glass_color, true, []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"),  []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_arms].metallic_color, true, []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"),  []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_arms].phong_amount, 0, 100, "%.0f", []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_arms].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[local_arms].enable && vars.visuals.chams[local_arms].material == 3;
							}));
					}

				}
				other_esp_main->add_element(new c_checkbox(str("Weapons"), &vars.visuals.chams[local_weapon].enable)); {
					auto local_weapons_enabled = []() { return vars.visuals.chams[local_weapon].enable; };

					other_esp_main->add_element(new c_combo(str("Material"), &vars.visuals.chams[local_weapon].material,
						{
							str("Default"),
							str("Flat"),
							str("Glass"),
							str("Metallic")
						}, local_weapons_enabled));

					other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].material_color, true, local_weapons_enabled));
					other_esp_main->add_element(new c_text(str("Material color"),  local_weapons_enabled));

					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].glass_color, true, []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 2;
							}));

						other_esp_main->add_element(new c_text(str("Glass color"),  []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 2;
							}));
					}
					{
						other_esp_main->add_element(new c_colorpicker(&vars.visuals.chams[local_weapon].metallic_color, true, []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
							}));

						other_esp_main->add_element(new c_text(str("Metallic color"), []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Phong amount"), &vars.visuals.chams[local_weapon].phong_amount, 0, 100,"%.0f", []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
							}));

						other_esp_main->add_element(new c_slider(str("Rim amount"), &vars.visuals.chams[local_weapon].rim_amount, -100, 100, "%.0f", []() { return
							vars.visuals.chams[local_weapon].enable && vars.visuals.chams[local_weapon].material == 3;
							}));
					}
				}

				


				
					
				
			
				other_esp_main->initialize_elements();
			}
			window->add_element(other_esp_main);
		}
		ImGui::Spacing();
		
		window->add_tab(new c_tab("D", tab_t::world, window)); {

			auto trace_child = new c_child("Miscellaneous", tab_t::world, window);
			trace_child->set_size(Vector2D(255, 260)); 
			trace_child->set_position(Vector2D(372, 250)); {

				trace_child->add_element(new c_checkbox(str("Zeus bot"), &vars.ragebot.zeusbot,
					enable_rage));

				trace_child->add_element(new c_slider(str("Zeus hitchance"), &vars.ragebot.zeuschance,
					0.f, 100.f, "%.0f", []() {return enable_rage() && vars.ragebot.zeusbot; }));
				trace_child->add_element(new c_checkbox(str("Auto jump"),
					&vars.misc.bunnyhop));

				trace_child->add_element(new c_checkbox(str("Auto strafe"),
					&vars.misc.autostrafe));

				trace_child->add_element(new c_checkbox(str("Slide walk"),
					&vars.misc.slidewalk));

				trace_child->add_element(new c_keybind(str("Peek assistance"), &g_Binds[bind_peek_assist]));
				trace_child->add_element(new c_checkbox(str("Buy-bot"),
					&vars.misc.autobuy.enable));

				trace_child->add_element(new c_combo(str("Primary weapon"),
					&vars.misc.autobuy.main, {
						str("None"),
						str("Auto sniper"),
						str("Scout"),
						str("Awp"),
						str("Negev"),
						str("M249"),
						str("Rifle"),
						str("AUG/SG553"),
					}, []() { return vars.misc.autobuy.enable; }));

				trace_child->add_element(new c_combo(str("Pistol"),
					&vars.misc.autobuy.pistol, {
						str("None"),
						str("Dual berettas"),
						str("P250"),
						str("CZ75-Auto"),
						str("Deagle/Revolver")
					}, []() { return vars.misc.autobuy.enable; }));

				trace_child->add_element(new c_multicombo(str("Extra"),
					&vars.misc.autobuy.misc, {
						str("Head helmet"), // 1
						str("Other helmet"), // 2
						str("HE grenade"), // 4
						str("Molotov"), // 8
						str("Smoke"), // 16
						str("Taser"), // 32
						str("Defuse kit"), // 64
					}, []() { return vars.misc.autobuy.enable; }));


				trace_child->add_element(new c_checkbox(str("Ragdoll force"),
					&vars.visuals.ragdoll_force));

				trace_child->add_element(new c_checkbox(str("Clantag changer"),
					&vars.visuals.clantagspammer));

				trace_child->add_element(new c_checkbox(str("Knife-bot"),
					&vars.misc.knifebot));

				trace_child->initialize_elements();
			}
			window->add_element(trace_child);

			auto effects_child = new c_child("Settings", tab_t::world, window);
			effects_child->set_size(Vector2D(255, 260));
			effects_child->set_position(Vector2D(372, -40)); {

				effects_child->add_element(new c_combo(str("Cheat mode"), &vars.misc.restrict_type, {
					str("Matchmaking"),
					str("HvH")
					}, nullptr));

				effects_child->add_element(new c_checkbox(str("Anti-untrusted"),
					&vars.misc.antiuntrusted, []() {
						return vars.misc.restrict_type == 1;
					}));

				
				effects_child->add_element(new c_button("Unload", []() { csgo->DoUnload = true; }));
				effects_child->add_element(new c_button(str("Full update"), []() { csgo->client_state->ForceFullUpdate(); }));

				effects_child->add_element(new c_button(str("Spoof sv_cheats"), []() {
					ConVar* sv_cheats = interfaces.cvars->FindVar(hs::sv_cheats.s().c_str());
					*(int*)((DWORD)&sv_cheats->m_fnChangeCallbacks + 0xC) = 0;
					sv_cheats->SetValue(1);
					}, []() { return vars.misc.restrict_type == 1; }));

				effects_child->add_element(new c_button(str("Unlock developer's convars"), EnableHiddenCVars, []() { return vars.misc.restrict_type == 1; }));
				
				effects_child->initialize_elements();
			}
			window->add_element(effects_child);
			reinit_config();
		}
		ImGui::Spacing();
		window->add_tab(new c_tab("E", tab_t::skins, window)); {

			auto sk_child = new c_child("tracers", tab_t::skins, window);
			sk_child->set_size(Vector2D(255, 500));
			sk_child->set_position(Vector2D(70, -50)); {

				sk_child->initialize_elements();
			}
			window->add_element(sk_child);

			auto skin_child = new c_child("effects", tab_t::skins, window);
			skin_child->set_size(Vector2D(255, 500));
			skin_child->set_position(Vector2D(362, -50)); {


				skin_child->initialize_elements();
			}
			window->add_element(skin_child);
		}
		ImGui::Spacing();
		window->add_tab(new c_tab("F", tab_t::misc, window)); {
			auto main_child = new c_child("main", tab_t::misc, window);
			main_child->set_size(Vector2D(255, 250));
			main_child->set_position(Vector2D(70, -50)); {

			
				main_child->initialize_elements();
			}
			window->add_element(main_child);

			auto other_child = new c_child("misc", tab_t::misc, window);
			other_child->set_size(Vector2D(255, 240));
			other_child->set_position(Vector2D(70, 210)); {
				

				other_child->initialize_elements();
			}
			window->add_element(other_child);


		}



		window->set_active_tab_index(tab_t::rage);
		initialized = true;
	}
	if (!vars.menu.open)
	{
		if (window->get_transparency() > 0.f)
			window->decrease_transparency(animation_speed * 80.f);
	}
	else
	{
		if (window->get_transparency() > 0.f)
			window->increase_transparency(animation_speed * 80.f);
	}
	ImGui::GetIO().MouseDrawCursor = window->get_transparency() > 0.f;
	POINT mp;
	GetCursorPos(&mp);
	ScreenToClient(GetForegroundWindow(), &mp);
	g_mouse.x = mp.x;
	g_mouse.y = mp.y;
	if (should_reinit_config) {
		reinit_config();
		should_reinit_config = false;
	}
	window->render();
	if (window->g_hovered_element) {
		if (window->g_hovered_element->type == c_elementtype::input_text)
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
	}
	csgo->scroll_amount = 0;

	g_Render->_drawList->Flags = flags_backup;
}
void CMenu::reinit_config() {
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "configs") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}

	RefreshConfigs();
	auto config_child = new c_child("configs", tab_t::world, window);
	config_child->set_size(Vector2D(255, 550));
	config_child->set_position(Vector2D(92, -40)); {
		config_child->add_element(new c_listbox("configs", &vars.menu.active_config_index, ConfigList, 150.f));
		config_child->add_element(new c_input_text("config name", &vars.menu.active_config_name, false));

		config_child->add_element(new c_button("load", []() {
			Config.Load(ConfigList[vars.menu.active_config_index]);
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		config_child->add_element(new c_button("save", []() {
			Config.Save(ConfigList[vars.menu.active_config_index]);
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		config_child->add_element(new c_button("refresh", []() { g_Menu->should_reinit_config = true; }));

		config_child->add_element(new c_button("create", []() {
			string add;
			if (vars.menu.active_config_name.find(".cfg") == -1)
				add = ".cfg";
			Config.Save(vars.menu.active_config_name + add);
			g_Menu->should_reinit_config = true;
			vars.menu.active_config_name.clear();
			}));

		config_child->add_element(new c_button("reset to default", []() { Config.ResetToDefault(); },
			[]() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		config_child->initialize_elements();
	}
	window->add_element(config_child);
}
void CMenu::update_binds()
{
	if (GetForegroundWindow() != csgo->Init.Window)
		return;

	if (g_Misc->IsChatOpened())
		return;
	csgo->mtx.lock();
	for(int i = 0; i < bind_max; ++i)
	{
		auto binder = &g_Binds[i]; 
		if (i == bind_manual_left || i == bind_manual_right || i == bind_manual_back || i == bind_manual_forward) {
			if (!vars.antiaim.manual_antiaim || !vars.antiaim.enable) {
				binder->active = false;
				continue;
			}
			binder->type = 2;
			if (binder->key > 0 && window->key_updated(binder->key)) {
				if (i == bind_manual_left) {
					binder->active = !binder->active;
					if (binder->active) {
						g_Binds[bind_manual_back].active = false;
						g_Binds[bind_manual_right].active = false;
						g_Binds[bind_manual_forward].active = false;
					}
				}
				else if (i == bind_manual_right) {
					binder->active = !binder->active;
					if (binder->active) {
						g_Binds[bind_manual_back].active = false;
						g_Binds[bind_manual_left].active = false;
						g_Binds[bind_manual_forward].active = false;
					}
				}
				else if (i == bind_manual_back) {
					binder->active = !binder->active;
					if (binder->active) {
						g_Binds[bind_manual_left].active = false;
						g_Binds[bind_manual_right].active = false;
						g_Binds[bind_manual_forward].active = false;
					}
				}
				else if (i == bind_manual_forward) {
					binder->active = !binder->active;
					if (binder->active) {
						g_Binds[bind_manual_left].active = false;
						g_Binds[bind_manual_back].active = false;
						g_Binds[bind_manual_right].active = false;
					}
				}
			}
		}
		else
		{
			binder->key = std::clamp<unsigned int>(binder->key, 0, 255);

			if (binder->type == 2 && binder->key > 0) {
				if (window->key_updated(binder->key)) {
					binder->active = !binder->active;
				}
			}
			else if (binder->type == 1 && binder->key > 0) {
				binder->active = csgo->key_pressed[binder->key];
			}
			else if (binder->type == 3 && binder->key > 0) {
				binder->active = !csgo->key_pressed[binder->key];
			}
			else if (binder->type == 0)
				binder->active = false;
			else if (binder->type == 4)
				binder->active = true;
		}
	}
	
	if (vars.misc.restrict_type == 0) {
		g_Binds[bind_hide_shots].active = false;
		g_Binds[bind_double_tap].active = false;
	
		g_Binds[bind_fake_duck].active = false;
	}

	csgo->mtx.unlock();
}
