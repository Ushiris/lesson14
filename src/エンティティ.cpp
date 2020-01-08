﻿#include <algorithm>
#include "エンティティ.h"
#include "サービス・レンダリング.h"
#include "DxLib.h"

namespace エンジン
{
	/// static メンバー
	システムサービス* コンポーネント::システムサービス_ = nullptr;
	システムサービス* エンティティ::システムサービス_ = nullptr;

	/////////////////////////////////////////////////////
	/// コンポーネント
	/////////////////////////////////////////////////////

	コンポーネント::コンポーネント(エンティティ& 親):親_(親)
	{
	}

	コンポーネント::~コンポーネント()
	{
	}
	
	bool コンポーネント::キャスト可能？(const コンポーネント* インスタンス, const TCHAR* 名前)
	{
		if (_tcscmp(L"スプライトコンポーネント", 名前) == 0) { return nullptr != dynamic_cast<const スプライトコンポーネント*>(インスタンス); }
		if (_tcscmp(L"入力コンポーネント", 名前) == 0) { return nullptr != dynamic_cast<const 入力コンポーネント*>(インスタンス); }
		if (_tcscmp(L"弾丸コンポーネント", 名前) == 0) { return nullptr != dynamic_cast<const 弾丸コンポーネント*>(インスタンス); }
		if (_tcscmp(L"CircleTrigger", 名前) == 0) { return nullptr != dynamic_cast<const CircleTrigger*>(インスタンス); }

		return false;
	}

	コンポーネント* コンポーネント::コンポーネント生成(const TCHAR* 名前, エンティティ& 親)
	{
		struct 文字列分岐 { TCHAR* 文字列; コンポーネント* (*関数)(エンティティ& 親); };

		static 文字列分岐 対応表[] =
		{
			// 具象クラスのコンポーネントが追加されたらここにデータを追加
			{ L"スプライトコンポーネント",   [](エンティティ& 親) { return (コンポーネント*)(new スプライトコンポーネント(親)); } },
			{ L"入力コンポーネント",         [](エンティティ& 親) { return (コンポーネント*)(new 入力コンポーネント(親)); } },
			{ L"弾丸コンポーネント",         [](エンティティ& 親) { return (コンポーネント*)(new 弾丸コンポーネント(親)); } },
			{ L"CircleTrigger",				 [](エンティティ& 親) { return (コンポーネント*)(new CircleTrigger(親)); } },
		};

		for(const auto &c : 対応表)
		{
			int diff = _tcscmp(c.文字列, 名前);
			if(diff == 0) {return (*c.関数)(親);}
		}

		return nullptr;
	}


	/////////////////////////////////////////////////////
	// エンティティ
	/////////////////////////////////////////////////////

	エンティティ::エンティティ()
	{
		位置_.x = 位置_.y = 0.0f;
	}

	エンティティ::~エンティティ()
	{
		コンポーネントの全削除();
	}

	コンポーネント* エンティティ::コンポーネント検索(const TCHAR* 名前)
	{
		for (auto& コンポーネント : コンポーネント配列_) {
			if(コンポーネント::キャスト可能？(コンポーネント, 名前)) return コンポーネント;
		}

		return nullptr;
	}


	void エンティティ::コンポーネントの全削除()
	{
		auto コンポーネント = コンポーネント配列_.begin();
		while (コンポーネント != コンポーネント配列_.end()) {
			安全DELETE(*コンポーネント);
			コンポーネント = コンポーネント配列_.erase(コンポーネント);
		}
	}

	void エンティティ::更新処理(float 経過時間)
	{
		// コンポーネントを最初に処理
		for (auto& コンポーネント : コンポーネント配列_) {
			コンポーネント->更新(経過時間);
		}

		// コンポーネントの後に自分の更新の処理
		更新(経過時間);
	}


	/////////////////////////////////////////////////////
	// エンティティ・システム
	/////////////////////////////////////////////////////

	エンティティサービス::エンティティサービス()
	{
	}

	エンティティサービス::~エンティティサービス()
	{
		全削除();
	}

	int エンティティサービス::初期化(システムサービス *サービス)
	{
		コンポーネント::システムサービス_ = サービス;
		エンティティ::システムサービス_ = サービス;

		return 0;
	}


	bool エンティティサービス::キャスト可能？(エンティティ* インスタンス, 種類 種類)
	{
		switch (種類) {
		case 種類::プレイヤー: return nullptr != dynamic_cast<const プレイヤー・エンティティ*>(インスタンス);
		case 種類::ステージ１: return nullptr != dynamic_cast<const ステージ１・エンティティ*>(インスタンス);
		case 種類::ザコ１:     return nullptr != dynamic_cast<const ザコ１・エンティティ*>(インスタンス);
		default:return false; // おかしな種類が指定された
		}
		return false;
	}

	int エンティティサービス::追加(種類 種類)
	{
		エンティティ* p = nullptr;

		switch (種類) {
		case 種類::プレイヤー:
			p = new プレイヤー・エンティティ();
			break;
		case 種類::ステージ１:
			p = new ステージ１・エンティティ();
			break;
		case 種類::ザコ１:
			p = new ザコ１・エンティティ();
			break;
		default:
			return -1; // おかしな種類が指定された
		}

		エンティティマップ_[エンティティID_++] = p;

		return エンティティID_ - 1;
	}

	エンティティ* エンティティサービス::エンティティ取得(int ハンドル)
	{
#ifdef _DEBUG
		// 存在確認
		auto itr = エンティティマップ_.find(ハンドル);
		if (itr == エンティティマップ_.end()) return nullptr;// 登録されていない
#endif // _DEBUG

		return エンティティマップ_[ハンドル];
	}


	エンティティ* エンティティサービス::最初のエンティティ検索(種類 種類)
	{
		auto it = エンティティマップ_.begin();
		while (it != エンティティマップ_.end()) {
			if (キャスト可能？(it->second, 種類)) {
				return it->second;
			}
			it++;
		}

		return nullptr;
	}


	int エンティティサービス::全削除()
	{
		auto it = エンティティマップ_.begin();
		while (it != エンティティマップ_.end()) {
			安全DELETE(it->second);
			エンティティマップ_.erase(it++);
		}

		return 0;
	}

	int エンティティサービス::削除(int ハンドル)
	{
		auto itr = エンティティマップ_.find(ハンドル);
		if (itr == エンティティマップ_.end()) return -1;// 既に登録されていない

		delete エンティティマップ_[ハンドル];
		エンティティマップ_.erase(ハンドル);

		return 0;
	}

	void エンティティサービス::更新(float 経過時間)
	{
		for (auto it = エンティティマップ_.begin(); it != エンティティマップ_.end(); it++)
		{
			it->second->更新処理(経過時間);
		}
	}

	void エンティティサービス::描画()
	{
		for (auto it = エンティティマップ_.begin();it != エンティティマップ_.end(); it++)
		{
			it->second->描画();
		}
	}

	void CircleTrigger::更新(float 経過時間)
	{
		
	};

	float2 CircleTrigger::getPos()
	{
		if (isBullet)
		{
			return *pos;
		}

		return 親_.位置取得();
	}
	int CircleTrigger::getR()
	{
		return r;
	}
	void CircleTrigger::ChengeLayer(int NewLayer)
	{
		layer = NewLayer;
	}
	void CircleTrigger::setBulletMode(float2 * NewPos)
	{
		isBullet = true;
		pos = NewPos;
	}
	bool CircleTrigger::collision(CircleTrigger another)
	{
		float a = getPos().x - another.getPos().x;
		float b = getPos().y - another.getPos().y;
		float c = a * a + b * b;
		float sum_radius = r + another.getR();

		if (c <= sum_radius * sum_radius)
		{
			return true;
		}

		return false;
	}
	bool CircleTrigger::isCollision()
	{
		for (auto another : collider)
		{
			if (collision(*another)||another->layer!=layer)
			{
				return true;
			}
		}
		return false;
	}
}
