# Основа материалов поверхностей

Путь в Content Browser: `/Game/DroneTest/Materials`.

- `Masters/MM_Backrooms` — мастер Surface/Opaque, один базовый слой по умолчанию.
- `Layers/ML_Surface` — базовый слой, адаптированный из ML_Yellow_Wall проекта AnomalyLoop.
- `Layers/MLB_SurfaceBlend` — опциональное смешивание двух слоёв по весу, маске и vertex color.
- `Instances` — папка для ваших Material Instance; старых экземпляров здесь нет.

Три ассета перенесены из пользовательского AnomalyLoop, не созданы с нуля. Текстуры Backrooms, ковра, дерева, готовые MI/MLI, лампа, бутылка и карты не переносились. Нейтральные заглушки — ресурсы Engine; в папке материалов нет собственных текстур. Существующая карта не менялась.

## Создать свой материал

1. ПКМ по `Masters/MM_Backrooms` → Create Material Instance. Переместите новый `MI_...` в `Instances`.
2. Откройте экземпляр. Во вкладке **Layer Parameters** раскройте базовый слой `ML_Surface` (это параметры слоя, а не глобальные параметры Details).
3. Включите override нужных параметров и назначьте свои `BaseColor_Texture`, `Normal_Texture`, `Roughness_Texture`.
4. BaseColor обычно sRGB включён; Normal — Normalmap compression и sRGB выключен; Roughness — sRGB выключен, sampler должен соответствовать исходному линейному слоту. Не назначайте normal как обычную цветовую карту.
5. `Tiling` — частота повторения; `Base_Tint` — оттенок; `Roughness_Multiplier` — множитель шероховатости; `Metallic` — металличность.
6. `Normal_Flatness`: 0 сохраняет normal, 1 делает её плоской. Вход ограничен Saturate(0..1), в том числе при ручном вводе за пределами слайдера.
7. Сохраните экземпляр и назначьте его нужному mesh. Автоматически на карту ничего не назначается.

## Добавить слой позже

В Layer Parameters включите override стека, добавьте слой `ML_Surface` и выберите `MLB_SurfaceBlend` между ним и нижним слоем. Настройки текстур и цвета каждого слоя независимы.

Параметры blend: `Blend_Weight`, `Blend_Intensity`, `BlendMask_Texture`, `Mask_Intensity`, `Use_Vertex_Color`, `Color_Channel`. При выключенном Use_Vertex_Color итоговый вес — Saturate(Blend_Weight * Blend_Intensity + mask * Mask_Intensity). По умолчанию Mask_Intensity=0: старой macro-текстуры нет, белая заглушка не добавляет вклад. Для маски настройте линейную текстуру в подходящем формате.

Для простых поверхностей оставляйте один слой. Дополнительные слои увеличивают стоимость шейдера. Эта основа не содержит стекла, прозрачности или постпроцесса VHS.

## Проверки

`Scripts/VerifySurfaceMaterials.py` запускается в Unreal с PythonScriptPlugin, DX12 и AllowCommandletRendering. Проверяет три сохранённых ассета, отсутствие чужих /Game-зависимостей, ограничение flatness, создание несохранённого MI, назначение/сброс override и компиляцию двухслойного расширения. Проверочные ассеты в Content не сохраняет.

`Scripts/ImportSurfaceMaterials.py` — одноразовая процедура переноса с отказом при существующих целевых ассетах. Для обычной работы повторно запускать её не нужно. Исходные три пакета временно размещались в их старом пути только на время переноса, затем удалены после сверки хешей с AnomalyLoop.
