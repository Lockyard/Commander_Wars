#include "3rd_party/oxygine-framework/oxygine/tween/TweenAlphaFade.h"
#include "3rd_party/oxygine-framework/oxygine/STDRenderDelegate.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"

namespace oxygine
{
    class TweenAlphaFadeImpl : public TweenPostProcess
    {
    public:
        TweenAlphaFadeImpl(bool fadeIn, const PostProcessOptions& opt) : TweenPostProcess(opt), _fadeIn(fadeIn) {}

        void render(Actor*, const RenderState&)
        {
            if (!_pp._rt)
            {
                return;
            }
            spSTDMaterial mat = new STDMaterial;
            mat->_base = _pp._rt;
            mat->_blend = blend_premultiplied_alpha;
            mat->apply();

            int _a = lerp(_fadeIn ? 0 : 255, _fadeIn ? 255 : 0, _progress);
            STDRenderer* renderer = STDRenderer::getCurrent();

            spNativeTexture rt = _pp._rt;
            RectF src(0, 0,
                      _pp._screen.getWidth() / (float)rt->getWidth(),
                      _pp._screen.getHeight() / (float)rt->getHeight());
            RectF dest = _pp._screen.cast<RectF>();



            AffineTransform tr = _pp._transform * _actor->computeGlobalTransform();
            renderer->setTransform(tr);


            QColor color = QColor(Qt::white);
            color.setAlpha(_a);
            renderer->addQuad(qRgba(premultiply(color)), src, dest);
        }

        bool _fadeIn;
    };


    TweenAlphaFade::TweenAlphaFade(bool fadeIn, const PostProcessOptions& opt): TweenProxy(new TweenAlphaFadeImpl(fadeIn, opt))
    {
    }
}
