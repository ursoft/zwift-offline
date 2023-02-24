#pragma once
struct GNViewBeginParams {};
struct GNViewInitializeParams {
    GNScene *m_scene;
    int m_w, m_h;
    uint16_t m_field_10;
};
class GNView {
public:
    GNView();
    ~GNView();
    void Begin(const GNViewBeginParams &);
    void Initialize(const GNViewInitializeParams &);
    void RenderCubeMap(RenderTarget *, const GNViewBeginParams &);
    void End();
};
inline GNView g_ctMainView, g_ctReflView, g_PreviewView;