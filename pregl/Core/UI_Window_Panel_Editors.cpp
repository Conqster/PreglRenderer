#include "UI_Window_Panel_Editors.h"


#include "Renderer/Shader.h"
#include "Renderer/Lighting.h"
#include <glm/glm.hpp>
#include "Renderer/GPUResources.h"
#include "Core/EditorCamera.h"
#include "Renderer/Material.h"
#include <array>
#include "Renderer/GPUVertexData.h"

#include <libs/imgui/imgui.h>
#include "Core/Util.h"
#include "Loader/Loader.h"

#include "Core/Log.h"


enum class PreviewMeshType : uint8_t
{
	SPHERE,
	QUAD,
	CUBE,

	MAX_PREVIEW_MESHES
};
//find a centralised location to this data, when memory starts becoming and issue; for sharing resources with other systems
struct Resources
{
	Shader previewShader;
	std::array<RenderableMesh, static_cast<size_t>(PreviewMeshType::MAX_PREVIEW_MESHES)> previewMeshes;

	GPUResource::UniformBuffer cameraUBO;
	GPUResource::Texture errorTexture;

	bool bMaterialEditorInit = false;
	GPUResource::Framebuffer materialPreviewFBO;
	std::weak_ptr<BaseMaterial> preview_material_ptr;

public:

	~Resources() {
		//ShutDown();
	}

	void Initialise()
	{
		previewMeshes[0] = Util::CreateSphere();
		previewMeshes[1] = Loader::LoadMesh("assets/meshes/quad.rmesh");
		previewMeshes[2] = Loader::LoadMesh("assets/meshes/cube.rmesh");

		previewShader.Create("preview_shader", "assets/shaders/lighting/PreviewMaterial.vert", "assets/shaders/lighting/PreviewMaterial.frag");
		//------------------Camera Matrix Data UBO-----------------------------/
		long long int buf_size = sizeof(glm::vec3);//for view pos
		buf_size += sizeof(float);// camera far
		buf_size += 2 * sizeof(glm::mat4);// +sizeof(glm::vec2);   //to store view, projection
		cameraUBO.Generate(buf_size);
		int block_idx = 2;
		cameraUBO.BindBufferRndIdx(block_idx, buf_size, 0);
		previewShader.Bind();
		previewShader.SetUniformBlockIdx("uCameraMat", block_idx);


		errorTexture = GPUResource::Texture("assets/textures/Blank.png", false);
	}


	void OnMaterialInit(UI::Windows::MaterialList mat)
	{
		materialPreviewFBO.Generate(512, 512);
		preview_material_ptr = mat[0];
		bMaterialEditorInit = true;
	}


	void ShutDown()
	{
		previewShader.Clear();
		for (auto& m : previewMeshes)
			m.Clear();

		cameraUBO.Delete();
		errorTexture.Clear();

		if (bMaterialEditorInit)
		{
			materialPreviewFBO.Delete();
			bMaterialEditorInit = false;
		}
	}
};

static std::unique_ptr<Resources> gResources;

void UI::Initialise()
{
	gResources = std::make_unique<Resources>();
	gResources->Initialise();
}

void UI::ShutDown()
{
	gResources->ShutDown();
}




static std::vector<UI::UIFlag> gsUIOpenTrackers;

void UI::RegisterandGetUIFlag(const char* name, bool* p_open)
{
	gsUIOpenTrackers.push_back({ p_open, name });
	DEBUG_LOG_ERROR("Registered new UI Flag, name_buf: ", name);
}

std::vector<UI::UIFlag>& UI::GetRegisteredUIFlags()
{
	return gsUIOpenTrackers;
}


///////////////////////////////////////////////////////
//Utilities
///////////////////////////////////////////////////////

void UpdateCameraUBO(EditorCamera& cam, float aspect_ratio)
{
	//------------------Camera Matrix Data UBO-----------------------------/
	unsigned int offset_ptr = 0;
	gResources->cameraUBO.SetSubDataByID(&(cam.GetPosition()[0]), sizeof(glm::vec3), offset_ptr);
	offset_ptr += sizeof(glm::vec3);
	gResources->cameraUBO.SetSubDataByID(&cam.mFar, sizeof(float), offset_ptr);
	offset_ptr += sizeof(float);
	gResources->cameraUBO.SetSubDataByID(&(cam.ProjMat(aspect_ratio)[0][0]), sizeof(glm::mat4), offset_ptr);
	offset_ptr += sizeof(glm::mat4);
	gResources->cameraUBO.SetSubDataByID(&(cam.ViewMat()[0][0]), sizeof(glm::mat4), offset_ptr);

}

void MaterialShaderHelper(Shader& shader, const BaseMaterial& mat)
{
	//Material (u_Material)
//diffuse; vec3s
	shader.SetUniformVec3("uMaterial.diffuse", mat.diffuse);
	//ambient;
	shader.SetUniformVec3("uMaterial.ambient", mat.ambient);
	//specular;
	shader.SetUniformVec3("uMaterial.specular", mat.specular);
	//shinness; float
	shader.SetUniform1f("uMaterial.shinness", mat.shinness);
	shader.SetUniform1f("uOpacity", mat.opacity);
}


void UI::Windows::MaterialsEditor(MaterialList& materials)
{
	static bool update_preview_fbo = true;
	static int mesh_id = 0;
	static Lighting::Directional preview_lighting = Lighting::Directional(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f), glm::vec3(1.0), glm::vec3(1.0f));
	static glm::mat4 preview_transform = glm::mat4(1.0f);
	static EditorCamera matertial_preview_cam(glm::vec3(0.0f, 0.0f, -1.9f), 90.0f, 0.0f, 100.0f, 5.0f);
	static float mPreviewAmbientRatio = 0.4f;

	if (!gResources->bMaterialEditorInit)
		gResources->OnMaterialInit(materials);

	update_preview_fbo = true;
	if (update_preview_fbo)
	{
		gResources->materialPreviewFBO.Bind();
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Update camera 
		float aspect_ratio = gResources->materialPreviewFBO.GetWidth() / gResources->materialPreviewFBO.GetHeight();
		UpdateCameraUBO(matertial_preview_cam, aspect_ratio);
		//tranform matrix later
		gResources->previewShader.Bind();
		//with the hope that other parameter are updated like 
		//the lighting in  
		gResources->previewShader.SetUniformMat4("uModel", preview_transform);
		//m_Shader.SetUniform1i("u_OnlyBaseColour", !b_MaterialPreviewLighting);
		gResources->previewShader.SetUniform1i("uPhongRendering", !gResources->preview_material_ptr.lock()->blinn_phong);
		//---------directional Light-------------------
		gResources->previewShader.SetUniformVec3("uDirectionalLight.direction", preview_lighting.direction);
		//diffuse
		gResources->previewShader.SetUniformVec3("uDirectionalLight.diffuse", preview_lighting.base.diffuse);
		//ambient
		gResources->previewShader.SetUniformVec3("uDirectionalLight.ambient", preview_lighting.base.ambient);
		//specular
		gResources->previewShader.SetUniformVec3("uDirectionalLight.specular", preview_lighting.base.specular);
		gResources->previewShader.SetUniform1i("uDirectionalLight.enable", preview_lighting.base.enable);
		gResources->previewShader.SetUniform1f("uAmbientRatio", mPreviewAmbientRatio);
		if (gResources->preview_material_ptr.lock()->diffuseMap)
			gResources->preview_material_ptr.lock()->diffuseMap->Activate();
		else
			gResources->errorTexture.Activate();
		gResources->previewShader.SetUniform1i("uDirectionalLight.enable", preview_lighting.base.enable);
		MaterialShaderHelper(gResources->previewShader, *gResources->preview_material_ptr.lock());


		auto preview_mesh = gResources->previewMeshes.front();
		if (mesh_id < static_cast<int>(PreviewMeshType::MAX_PREVIEW_MESHES))
			preview_mesh = gResources->previewMeshes[mesh_id];

		preview_mesh.Draw();

		gResources->materialPreviewFBO.UnBind();
		update_preview_fbo = false;
		//set external after ui has been rendered
		//glViewport(0, 0, mDisplayManager->GetWidth(), mDisplayManager->GetHeight());
	}


	if (ImGui::Begin("Materials Editor"))
	{
		ImVec2 ui_win_size = ImGui::GetWindowSize();
		ImVec2 preview_panel_size = ImVec2(ui_win_size.x * 0.5f, ui_win_size.x * 0.5f);
		preview_panel_size.y *= (gResources->materialPreviewFBO.GetHeight() / gResources->materialPreviewFBO.GetWidth()); //invert

		ImVec2 top_left = ImGui::GetCursorPos();
		ImGui::Image((ImTextureID)(intptr_t)gResources->materialPreviewFBO.GetRenderTextureGPU_ID(),
			preview_panel_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		update_preview_fbo |= Panels::TransformQuaternion(preview_transform, top_left, preview_panel_size);


		ImGui::SameLine();
		ImGui::BeginChild("##mesh_type", ImVec2(preview_panel_size.x * 0.75f, preview_panel_size.y * 0.5f));
		const char* mesh_type[] = { "Sphere", "Quad", "Cube" };
		update_preview_fbo |= ImGui::Combo("Preview Mesh", &mesh_id, mesh_type, 3);
		ImGui::EndChild();

		//material properties
		auto& mat = gResources->preview_material_ptr.lock();
		ImGui::Text(mat->name);
		//update_preview_fbo |= ImGui::Checkbox("Preview Lighting or Base colour", &b_MaterialPreviewLighting);
		update_preview_fbo |= ImGui::Checkbox("Blinn-Phong", &mat->blinn_phong);
		update_preview_fbo |= ImGui::ColorEdit3("Diffuse", &mat->diffuse[0]);
		update_preview_fbo |= ImGui::ColorEdit3("Ambient", &mat->ambient[0]);
		update_preview_fbo |= ImGui::ColorEdit3("Specular", &mat->specular[0]);
		update_preview_fbo |= ImGui::SliderFloat("Opacity", &mat->opacity, 0.0f, 1.0f, "%.1f");
		update_preview_fbo |= ImGui::SliderFloat("Shinness", &mat->shinness, 8.0f, 256.0f, "%.0f");
		update_preview_fbo |= ImGui::SliderFloat("Ambinent Ratio", &mPreviewAmbientRatio, 0.0f, 1.0f, "%.1f");
		//diffuse map
		int tex_id = (mat->diffuseMap) ? mat->diffuseMap->GetID() : gResources->errorTexture.GetID();
		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
		ImGui::SameLine(); ImGui::Text("Diffuse Map");
		//normal map
		tex_id = (mat->normalMap) ? mat->normalMap->GetID() : gResources->errorTexture.GetID();
		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
		ImGui::SameLine(); ImGui::Text("Normal Map");
		//specular map
		tex_id = (mat->specularMap) ? mat->specularMap->GetID() : gResources->errorTexture.GetID();
		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
		ImGui::SameLine(); ImGui::Text("Specular Map");


		//list all materials
		for (auto& mat : materials)
		{
			if (ImGui::Button(mat.lock()->name))
			{
				DEBUG_LOG("Button Pressed: ", mat.lock()->name);
				gResources->preview_material_ptr = mat;
				update_preview_fbo |= true;
			}
		}

		if (ImGui::TreeNode("Preview DirectionalLight"))
		{
			update_preview_fbo |= ImGui::Checkbox("Enable", &preview_lighting.base.enable);
			update_preview_fbo |= ImGui::DragFloat3("Light Direction", &preview_lighting.direction[0], 0.1f, -1.0f, 1.0f);
			update_preview_fbo |= ImGui::ColorEdit3("Diffuse colour", &preview_lighting.base.diffuse[0]);
			update_preview_fbo |= ImGui::ColorEdit3("Ambient colour", &preview_lighting.base.ambient[0]);
			update_preview_fbo |= ImGui::ColorEdit3("Specular colour", &preview_lighting.base.specular[0]);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Preview Camera"))
		{
			static bool update_cam = false;

			update_cam |= ImGui::SliderFloat("FOV", &matertial_preview_cam.mFOV, 0.0f, 179.0f, "%.1f");
			update_cam |= ImGui::DragFloat("Near", &matertial_preview_cam.mNear, 0.1f, 0.1f, 50.0f, "%.1f");
			update_cam |= ImGui::DragFloat("Far", &matertial_preview_cam.mFar, 0.1f, 0.0f, 500.0f, "%.1f");

			glm::vec3 pos = matertial_preview_cam.GetPosition();
			if (update_cam |= ImGui::DragFloat3("Position", &pos[0], 0.1f))
				matertial_preview_cam.SetPosition(pos);

			update_cam |= ImGui::SliderFloat("Yaw", &matertial_preview_cam.mYaw, 0.0f, 360.0f);
			update_cam |= ImGui::SliderFloat("Pitch", &matertial_preview_cam.mPitch, 0.0f, 360.0f);

			//quick hack
			if (update_cam)
				matertial_preview_cam.SetPosition(pos);

			update_preview_fbo |= update_cam;

			ImGui::TreePop();
		}

	}
	ImGui::End();
}


#include "Core/HeapMemAllocationTracking.h"

//void UI::Windows::SingleTextureEditor(GPUResource::Texture& texture)
void UI::Windows::SingleTextureEditor(GPUResource::Texture& texture, const char* name, bool* open_flag)
{
	//OPEN_BLOCK_MEM_TRACKING_PROFILE(texture);

	//snprintf(name_buf, sizeof(name_buf), "Texture Editor Window GPU: %d {WIP}.", int(texture.GetID()));
	//HELPER_REGISTER_UIFLAG_MULTICALL(name_buf, open_flag);
	ImGui::PushID(&texture);
	//if (ImGui::Begin(name_buf.c_str(), &open_flag))
	if (ImGui::Begin(name, open_flag))
	{
		ImVec2 ui_win_size = ImGui::GetWindowSize();
		ImVec2 preview_texture_size = ImVec2(ui_win_size.x * 0.5f, ui_win_size.x * 0.5f);

		

		ImVec2 top_left = ImGui::GetCursorPos();

		//ImGui::PushID(&top_left);
		static ImVec2 test_uv0 = ImVec2(0, 0);
		static ImVec2 test_uv1 = ImVec2(1, 1);
		ImGui::SliderFloat2("UV0", &test_uv0[0], 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat2("UV1", &test_uv1[0], 0.0f, 1.0f, "%.2f");
		//ImGui::PopID();
		//ImGui::Image((ImTextureID)(intptr_t)texture.GetID(), preview_texture_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		//ImGui::Image((ImTextureID)(intptr_t)texture.GetID(), preview_texture_size);
		ImGui::Image((ImTextureID)(intptr_t)texture.GetID(), preview_texture_size, test_uv0, test_uv1);

		///*static*/ int curr_tex_type = (int)texture.GetType();
		///*static*/ int curr_img_format = (int)texture.GetFormat();

		char name_buf[64];
		snprintf(name_buf, sizeof(name_buf), "Texture GPU ID: %d.", texture.GetID());
		ImGui::Text(name_buf);
		snprintf(name_buf, sizeof(name_buf), "Texture  Size: %d x %d.", texture.GetWidth(), texture.GetHeight());
		ImGui::Text(name_buf);

		bool update_texture = false;
		//cache a copy of texture parameter 
		GPUResource::TextureParameter tex_parameter = texture.GetParameter();
		int curr_tex_type = static_cast<int>(tex_parameter.textureType);
		auto tex_types = GPUResource::Utilities::TextureTypesToStringArray();
		update_texture |= ImGui::Combo("Texture Type", &curr_tex_type, tex_types.data(), tex_types.size());
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "{WIP}: Some of the options below crashes!!!");
		int curr_img_format = static_cast<int>(tex_parameter.imgInternalFormat);
		auto tex_img_format = GPUResource::Utilities::ImgFormatToStringArray();
		update_texture |= ImGui::Combo("Texture Image Internal Format", &curr_img_format, tex_img_format.data(), tex_img_format.size());
		update_texture |= ImGui::Checkbox("Use similar Internal - Tex Format", &tex_parameter.useEqualFormat);
		int curr_tex_format = static_cast<int>(tex_parameter.format);
		if (!tex_parameter.useEqualFormat)
			update_texture |= ImGui::Combo("Texture Format", &curr_tex_format, tex_img_format.data(), tex_img_format.size());

		snprintf(name_buf, sizeof(name_buf), "Texture Wrap Mode: %s.", GPUResource::Utilities::TextureWrapModeToStringArray()[(int)tex_parameter.wrapMode]);
		ImGui::Text(name_buf);
		snprintf(name_buf, sizeof(name_buf), "Texture Filter Mode: %s.", GPUResource::Utilities::TextureFilterModeToStringArray()[(int)tex_parameter.filterMode]);
		ImGui::Text(name_buf);

		if (update_texture)
		{
			DEBUG_LOG_WARNING("[SingleTextureEditor]: Need to update texture");

			tex_parameter.textureType = (GPUResource::TextureType)curr_tex_type;
			tex_parameter.imgInternalFormat = (GPUResource::IMGFormat)curr_img_format;
			tex_parameter.format = (tex_parameter.useEqualFormat) ? tex_parameter.imgInternalFormat : (GPUResource::IMGFormat)curr_tex_format;

			if (GPUResource::Utilities::IsFormatFloatPoint(tex_parameter.imgInternalFormat))
				tex_parameter.pxDataType = GPUResource::DataType::FLOAT;
			//if (!texture.GetImgPath().empty())
			//	texture.GenerateFromFile(texture.GetImgPath().c_str(), true, tex_parameter);
			//else
				texture.Generate(texture.GetWidth(), texture.GetHeight(), nullptr, tex_parameter);

			update_texture = false;
		}

	}
	ImGui::End();
	ImGui::PopID();
	///CLOSE_BLOCK_MEM_TRACKING_PROFILE(texture);
}


void UI::Windows::RenderTargetViewport(GPUResource::Framebuffer& render_target)
{
	if (ImGui::Begin("Render Target Viewport"))
	{
		ImVec2 ui_win_size = ImGui::GetWindowSize();
		float target_img_width = ui_win_size.x * 0.75f;
		ImVec2 preview_panel_size = ImVec2(target_img_width, target_img_width);
		preview_panel_size.y  *= (static_cast<float>(render_target.GetHeight()) / static_cast<float>(render_target.GetWidth()));

		ImVec2 top_left = ImGui::GetCursorPos();
		ImGui::Image((ImTextureID)(intptr_t)render_target.GetRenderTextureGPU_ID(),
			preview_panel_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		char text_buf[64];
		snprintf(text_buf, sizeof(text_buf), "Texture  Size: %d x %d, aspect ratio: %.1f.", render_target.GetWidth(), render_target.GetHeight(), (static_cast<float>(render_target.GetWidth()) / static_cast<float>(render_target.GetHeight())));
		ImGui::Text(text_buf);

	}
	ImGui::End();
}

void UI::Windows::MultiRenderTargetViewport(GPUResource::MultiRenderTarget& multi_render_target)
{
	if (ImGui::Begin("Multi Render Target Viewport"))
	{
		ImVec2 ui_win_size = ImGui::GetWindowSize();
		float target_img_width = ui_win_size.x * 0.75f;
		ImVec2 preview_panel_size = ImVec2(target_img_width, target_img_width);
		preview_panel_size.y *= (static_cast<float>(multi_render_target.GetHeight()) / static_cast<float>(multi_render_target.GetWidth()));

		//ImVec2 top_left = ImGui::GetCursorPos();
		char text_buf[64];
		snprintf(text_buf, sizeof(text_buf), "Frame buffer  Size: %d x %d.", multi_render_target.GetWidth(), multi_render_target.GetHeight());
		ImGui::Text(text_buf);
		for (auto& t : multi_render_target.GetRenderTargetTextures())
		{
			ImGui::Image((ImTextureID)(intptr_t)t.GetID(), preview_panel_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::Text("GPU ID: %d", t.GetID());
		}


	}
	ImGui::End();
}


bool UI::Panels::TransformQuaternion(glm::mat4& transform, ImVec2 top_left, ImVec2 size)
{
	glm::quat quat = glm::quat_cast(transform);
	ImVec2 ui_win_size = ImGui::GetWindowSize();
	ImVec2 drag_panel_size = ImVec2(ui_win_size.x * 0.5f, ui_win_size.x * 0.5f);
	static float drag_senstitivity = 0.001f;
	static int drag_speed = 5;
	static bool invert_x = false;
	static bool invert_y = false;


	//need location
	ImGui::SetCursorPos(top_left);
	ImGui::InvisibleButton("##quat_editor", size);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	draw_list->AddRect(min, max, IM_COL32(255, 255, 255, 255));


	//mouse drag
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
	{
		draw_list->AddRect(min, max, IM_COL32(0, 0, 255, 255));
		ImVec2 dt_drag = ImGui::GetMouseDragDelta();

		//dt drag -> rot angle
		float dt_x = dt_drag.x * drag_senstitivity * drag_speed;// *(invert_x) ? -1.0f : 1.0f;
		float dt_y = dt_drag.y * drag_senstitivity * drag_speed;

		dt_x *= (invert_x) ? -1.0f : 1.0f;
		dt_y *= (invert_y) ? 1.0f : -1.0f;

		glm::quat rot_x = glm::angleAxis(dt_y, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat rot_y = glm::angleAxis(dt_x, glm::vec3(0.0f, 1.0f, 0.0f));

		quat = glm::normalize(rot_y * rot_x * quat);

		transform = glm::mat4_cast(quat);

		ImGui::ResetMouseDragDelta();
		return true;
	}

	return false;
}
