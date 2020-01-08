﻿#pragma once
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include "共通.h"
#include "サービス・レンダリング.h"
#include "サービス・入力.h"
#include "サービス・弾丸.h"

namespace エンジン 
{
	// 前方宣言
	class エンティティ;
	class コンポーネント;
	class エンティティサービス;
	class レンダリングサービス;
	class 入力サービス;
	struct 入力データ;
	class 弾丸サービス;

	/////////////////////////////////////////////////////
	// コンポーネントから使うシステムサービスの集約
	/////////////////////////////////////////////////////
	class システムサービス
	{
	private:
		エンティティサービス& エンティティサービス_;
		レンダリングサービス& レンダリングサービス_;
		入力サービス& 入力_;
		弾丸サービス& 弾丸_;

	public:
		システムサービス(エンティティサービス& エンティティサービス, レンダリングサービス& レンダラー, 入力サービス& 入力, 弾丸サービス& 弾丸):
			エンティティサービス_(エンティティサービス),
			レンダリングサービス_(レンダラー), 
			入力_(入力), 
			弾丸_(弾丸){}
		~システムサービス() {}

		// エンティティから呼び出し
		エンティティサービス& エンティティ取得() { return エンティティサービス_; }
		レンダリングサービス& レンダラー取得() { return レンダリングサービス_; }
		入力サービス& 入力取得() { return 入力_; }
		弾丸サービス& 弾丸取得() { return 弾丸_; }
	};


	/////////////////////////////////////////////////////
	// コンポーネント
	/////////////////////////////////////////////////////
	// コンポーネントを追加したら、次も修正すること
	// + それぞれの名前
	// + コンポーネント::キャスト可能？ の中身
	// + コンポーネント::コンポーネント生成 の中身

	class コンポーネント 
	{
	public:// friend 宣言
		friend エンティティサービス;

	private:
		TCHAR *名前_ = L"なし";
	protected:
		static システムサービス* システムサービス_;
		エンティティ& 親_;

	public:
		コンポーネント(エンティティ& 親);
		virtual ~コンポーネント();

		static bool キャスト可能？(const コンポーネント *インスタンス, const TCHAR* 名前);

		static const std::type_info& 型情報取得(const TCHAR* 名前);
		TCHAR* 名前取得() { return 名前_; }
		virtual void 更新(float 経過時間) = 0;

	public:// 静的関数
		static コンポーネント* コンポーネント生成(const TCHAR* 名前, エンティティ& 親);
	};


	class スプライトコンポーネント final : public コンポーネント
	{
	private:
		TCHAR* 名前_ = L"スプライトコンポーネント";

		int リソースID_ = RID_EXPLOSION_L;		// スプライトの指定
		bool 位置は中心？_ = true;
	public:
		スプライトコンポーネント(エンティティ& 親) :コンポーネント(親) {}
		~スプライトコンポーネント() {}

		void リソース設定(int リソースID) { リソースID_ = リソースID; }
		void 更新(float 経過時間) {};
		void 描画();
	};

	class 入力コンポーネント final : public コンポーネント
	{
	private:
		TCHAR* 名前_ = L"入力コンポーネント";
		入力サービス &入力_;
	public:
		入力コンポーネント(エンティティ& 親) : コンポーネント(親)
			, 入力_(システムサービス_->入力取得()){}
		~入力コンポーネント(){}

		const 入力データ &データ取得() { return 入力_.データ取得(); };

		void 更新(float 経過時間) {};
	};

	class CircleTrigger final : public コンポーネント
	{

	public:
		CircleTrigger(エンティティ& 親) : コンポーネント(親) { }
		~CircleTrigger() {}

		void 更新(float 経過時間);

		float2 getPos();
		int getR();
		void ChengeLayer(int NewLayer);
		void setBulletMode(float2 *pos);

		bool collision(CircleTrigger another);
		bool isCollision();

	private:
		TCHAR* 名前_ = L"CircleTrigger";
		int layer = 0;
		float2 *pos;
		int r = 1;
		bool isBullet = false;
	};

	class 弾丸コンポーネント final : public コンポーネント
	{
	private:
		TCHAR* 名前_ = L"弾丸コンポーネント";
		弾丸サービス& 弾丸_;
	public:
		弾丸コンポーネント(エンティティ& 親) : コンポーネント(親)
			, 弾丸_(システムサービス_->弾丸取得()) {}
		~弾丸コンポーネント() {}

		int 追加(弾丸サービス::種類 種類, float2 位置, float2 速度) { return 弾丸_.追加(種類, 位置, 速度); }
		std::vector<float2> getAllEnemyBullets();
		void 更新(float 経過時間) {};
	};



	/////////////////////////////////////////////////////
	// エンティティ
	/////////////////////////////////////////////////////
	// エンティティを追加したら、次も修正すること
	// + エンティティ・システム::種類
	// + エンティティ・システム::型情報取得
	// + エンティティ・システム::追加

	class エンティティ
	{
	public:// friend 宣言
		friend エンティティサービス;

	protected:
		static システムサービス* システムサービス_;

		float2 位置_;
		std::vector<コンポーネント*> コンポーネント配列_;

	private:
		void コンポーネントの全削除();
		void 更新処理(float 経過時間);

	public:
		エンティティ();
		virtual ~エンティティ();

		void 追加(コンポーネント* コンポ) { コンポーネント配列_.push_back(コンポ); }
		コンポーネント* コンポーネント検索(const TCHAR *名前);

		const float2& 位置取得() const {return 位置_;}
		void 位置設定(float2 x) { 位置_ = x; }

		virtual void 更新(float 経過時間) = 0; // 作らなきゃだめ
		virtual void 描画() {};  // 作らなくていい
	};


	/////////////////////////////////////////////////////
	// 具象エンティティ
	/////////////////////////////////////////////////////

	class プレイヤー・エンティティ final : public エンティティ
	{
	private:
		スプライトコンポーネント* スプライト_;
		入力コンポーネント* 入力_;
		弾丸コンポーネント* 弾丸_;
		CircleTrigger* collider_;

	public:
		プレイヤー・エンティティ();
		~プレイヤー・エンティティ();

		void 更新(float 経過時間) override;
		void 描画() override;
	};


	class ザコ１・エンティティ final : public エンティティ
	{
	private:
		スプライトコンポーネント* スプライト_;
		float 生存時間_;
		float 弾を撃つまでの時間_;
		int 残弾_;
	public:
		ザコ１・エンティティ();
		~ザコ１・エンティティ() {}

		void 更新(float 経過時間) override;
		void 描画() override;
	};


	class ステージ１・エンティティ final : public エンティティ
	{
	private:
		int 状態_;
		float 時間_;
		float 状態での時間_;

		void 状態を進める() { 状態_++;  状態での時間_ = 0.0f; }
	public:
		ステージ１・エンティティ();
		~ステージ１・エンティティ() {}

		void 更新(float 経過時間) override;
	};


	/////////////////////////////////////////////////////
	// エンティティの管理
	/////////////////////////////////////////////////////

	class エンティティサービス
	{
	public:// 定数
		enum class 種類 {
			プレイヤー,
			ザコ１,
			ステージ１,
		};

	private:
		int エンティティID_ = 0;
		std::unordered_map<int, エンティティ*> エンティティマップ_;

		static bool キャスト可能？(エンティティ* インスタンス, 種類 種類);
		int 全削除();
	public:
		エンティティサービス();
		~エンティティサービス();

		int 初期化(システムサービス *サービス);
		int 片付け() { return 0; }

		int 追加(種類 種類);// ポインタを直接渡さずハンドル(int)を渡すことで、内部管理のコンテナの自由度を増す
		エンティティ* エンティティ取得(int ハンドル);
		エンティティ* 最初のエンティティ検索(種類 種類);
		int 削除(int ハンドル);


		void 更新(float 経過時間);
		void 描画();
	};
}
