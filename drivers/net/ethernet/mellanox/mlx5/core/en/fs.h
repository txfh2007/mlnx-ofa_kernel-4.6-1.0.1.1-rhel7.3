/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/* Copyright (c) 2018 Mellanox Technologies. */

#ifndef __MLX5E_FLOW_STEER_H__
#define __MLX5E_FLOW_STEER_H__

enum {
	MLX5E_TC_FT_LEVEL = 0,
	MLX5E_TC_TTC_FT_LEVEL,
};

#ifdef HAVE_TC_FLOWER_OFFLOAD
struct mlx5e_tc_table {
	/* protects flow table */
	struct mutex			t_lock;
	struct mlx5_flow_table		*t;

	spinlock_t			ht_lock; /* protects ht */
	struct rhashtable               ht;

	spinlock_t mod_hdr_tbl_lock; /* protects mod_hdr_tbl */
#ifdef HAVE_TCF_PEDIT_TCFP_KEYS_EX
	DECLARE_HASHTABLE(mod_hdr_tbl, 8);
#endif
	spinlock_t hairpin_tbl_lock; /* protects hairpin_tbl */
	DECLARE_HASHTABLE(hairpin_tbl, 8);

	struct notifier_block     netdevice_nb;
};
#endif

struct mlx5e_flow_table {
	int num_groups;
	struct mlx5_flow_table *t;
	struct mlx5_flow_group **g;
};

struct mlx5e_l2_rule {
	u8  addr[ETH_ALEN + 2];
	struct mlx5_flow_handle *rule;
};

#define MLX5E_L2_ADDR_HASH_SIZE BIT(BITS_PER_BYTE)

struct mlx5e_vlan_table {
	struct mlx5e_flow_table		ft;
	DECLARE_BITMAP(active_cvlans, VLAN_N_VID);
#ifdef HAVE_NETIF_F_HW_VLAN_STAG_RX
	DECLARE_BITMAP(active_svlans, VLAN_N_VID);
#endif
	struct mlx5_flow_handle	*active_cvlans_rule[VLAN_N_VID];
#ifdef HAVE_NETIF_F_HW_VLAN_STAG_RX
	struct mlx5_flow_handle	*active_svlans_rule[VLAN_N_VID];
#endif
	struct mlx5_flow_handle	*untagged_rule;
	struct mlx5_flow_handle	*any_cvlan_rule;
	struct mlx5_flow_handle	*any_svlan_rule;
	bool			cvlan_filter_disabled;
};

struct mlx5e_l2_table {
	struct mlx5e_flow_table    ft;
	struct hlist_head          netdev_uc[MLX5E_L2_ADDR_HASH_SIZE];
	struct hlist_head          netdev_mc[MLX5E_L2_ADDR_HASH_SIZE];
	struct mlx5e_l2_rule	   broadcast;
	struct mlx5e_l2_rule	   allmulti;
	struct mlx5e_l2_rule	   promisc;
	bool                       broadcast_enabled;
	bool                       allmulti_enabled;
	bool                       promisc_enabled;
};

enum mlx5e_traffic_types {
	MLX5E_TT_IPV4_TCP,
	MLX5E_TT_IPV6_TCP,
	MLX5E_TT_IPV4_UDP,
	MLX5E_TT_IPV6_UDP,
	MLX5E_TT_IPV4_IPSEC_AH,
	MLX5E_TT_IPV6_IPSEC_AH,
	MLX5E_TT_IPV4_IPSEC_ESP,
	MLX5E_TT_IPV6_IPSEC_ESP,
	MLX5E_TT_IPV4,
	MLX5E_TT_IPV6,
	MLX5E_TT_ANY,
	MLX5E_NUM_TT,
	MLX5E_NUM_INDIR_TIRS = MLX5E_TT_ANY,
};

enum mlx5e_tunnel_types {
	MLX5E_TT_IPV4_GRE,
	MLX5E_TT_IPV6_GRE,
	MLX5E_NUM_TUNNEL_TT,
};

/* L3/L4 traffic type classifier */
struct mlx5e_ttc_table {
	struct mlx5e_flow_table  ft;
	struct mlx5_flow_handle	 *rules[MLX5E_NUM_TT];
	struct mlx5_flow_handle  *tunnel_rules[MLX5E_NUM_TUNNEL_TT];
};

/* NIC prio FTS */
enum {
	MLX5E_VLAN_FT_LEVEL = 0,
	MLX5E_L2_FT_LEVEL,
	MLX5E_DECAP_FT_LEVEL,
	MLX5E_TTC_FT_LEVEL,
	MLX5E_INNER_TTC_FT_LEVEL,
#ifdef CONFIG_MLX5_EN_ARFS
	MLX5E_ARFS_FT_LEVEL
#endif
};

enum {
	MLX5E_ENCAP_FT_LEVEL = 0,
};

#ifdef CONFIG_MLX5_EN_RXNFC

struct mlx5e_ethtool_table {
	struct mlx5_flow_table *ft;
	int                    num_rules;
};

#define ETHTOOL_NUM_L3_L4_FTS 7
#define ETHTOOL_NUM_L2_FTS 4

struct mlx5e_ethtool_steering {
	struct mlx5e_ethtool_table      l3_l4_ft[ETHTOOL_NUM_L3_L4_FTS];
	struct mlx5e_ethtool_table      l2_ft[ETHTOOL_NUM_L2_FTS];
	struct list_head                rules;
	int                             tot_num_rules;
};

void mlx5e_ethtool_init_steering(struct mlx5e_priv *priv);
void mlx5e_ethtool_cleanup_steering(struct mlx5e_priv *priv);
int mlx5e_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd);
int mlx5e_get_rxnfc(struct net_device *dev,
#ifdef HAVE_ETHTOOL_OPS_GET_RXNFC_U32_RULE_LOCS
		    struct ethtool_rxnfc *info, u32 *rule_locs);
#else
			   struct ethtool_rxnfc *info, void *rule_locs);
#endif
#else
static inline void mlx5e_ethtool_init_steering(struct mlx5e_priv *priv)    { }
static inline void mlx5e_ethtool_cleanup_steering(struct mlx5e_priv *priv) { }
#endif /* CONFIG_MLX5_EN_RXNFC */

#ifdef CONFIG_MLX5_EN_ARFS
#define ARFS_HASH_SHIFT BITS_PER_BYTE
#define ARFS_HASH_SIZE BIT(BITS_PER_BYTE)

struct arfs_table {
	struct mlx5e_flow_table  ft;
	struct mlx5_flow_handle	 *default_rule;
	struct hlist_head	 rules_hash[ARFS_HASH_SIZE];
};

enum  arfs_type {
	ARFS_IPV4_TCP,
	ARFS_IPV6_TCP,
	ARFS_IPV4_UDP,
	ARFS_IPV6_UDP,
	ARFS_NUM_TYPES,
};

struct mlx5e_arfs_tables {
	struct arfs_table arfs_tables[ARFS_NUM_TYPES];
	/* Protect aRFS rules list */
	spinlock_t                     arfs_lock;
	struct list_head               rules;
	int                            last_filter_id;
	struct workqueue_struct        *wq;
};

int mlx5e_arfs_create_tables(struct mlx5e_priv *priv);
void mlx5e_arfs_destroy_tables(struct mlx5e_priv *priv);
int mlx5e_arfs_enable(struct mlx5e_priv *priv);
int mlx5e_arfs_disable(struct mlx5e_priv *priv);
int mlx5e_rx_flow_steer(struct net_device *dev, const struct sk_buff *skb,
			u16 rxq_index, u32 flow_id);
#else
static inline int mlx5e_arfs_create_tables(struct mlx5e_priv *priv) { return 0; }
static inline void mlx5e_arfs_destroy_tables(struct mlx5e_priv *priv) {}
static inline int mlx5e_arfs_enable(struct mlx5e_priv *priv) { return -EOPNOTSUPP; }
static inline int mlx5e_arfs_disable(struct mlx5e_priv *priv) {	return -EOPNOTSUPP; }
#endif

struct mlx5e_decap_table {
	struct mlx5e_flow_table		ft;
	struct mlx5_flow_handle		*miss_rule;
	bool enabled;
};

struct mlx5e_sniffer;

struct mlx5e_flow_steering {
	struct mlx5_flow_namespace      *ns;
#ifdef CONFIG_MLX5_EN_RXNFC
	struct mlx5e_ethtool_steering   ethtool;
#endif
#ifdef HAVE_TC_FLOWER_OFFLOAD
	struct mlx5e_tc_table           tc;
#endif
	struct mlx5e_vlan_table         vlan;
	struct mlx5e_l2_table           l2;
	struct mlx5e_ttc_table          ttc;
	struct mlx5e_ttc_table          inner_ttc;
#ifdef CONFIG_MLX5_EN_ARFS
	struct mlx5e_arfs_tables        arfs;
#endif
	struct mlx5e_sniffer            *sniffer;

	struct mlx5e_decap_table        decap;
};

struct ttc_params {
	struct mlx5_flow_table_attr ft_attr;
	u32 any_tt_tirn;
	u32 indir_tirn[MLX5E_NUM_INDIR_TIRS];
	struct mlx5e_ttc_table *inner_ttc;
};

struct mlx5_encap_info {
	char  *encap_header;
	int    encap_size;
	u32    encap_id;
	int    encap_id_valid;
};

struct mlx5e_encap_table {
	struct mlx5e_flow_table  ft;
	struct mlx5_flow_handle *dont_encap_rule;
};

struct mlx5e_encap_context {
	/*VxLAN fields*/
	__be64	tun_id;
	/*L4 fields*/
	__be16	tp_dst;
	/*L3 fields*/
	__be32	src;
	__be32	dst;
	__u8	ttl;
	__u8	tos;
	__be16	frag_off;
	/*L2 fields*/
	unsigned char	mac_dest[ETH_ALEN];
	unsigned char	mac_source[ETH_ALEN];
	bool	is_vlan;
	int	vid;
	u16 vlan_proto;
	/*Control fields*/
	struct mlx5_flow_handle	*rule;
	struct mlx5_encap_info	info;
	u32 flow_tag;
	int ref_count;
};

struct mlx5e_encap_context_table {
	struct mutex lock;
	struct mlx5e_encap_context data[];
};

struct mlx5e_tx_steering {
	struct mlx5e_encap_context_table *encap_context_table;
	struct mlx5e_encap_table    encap;
};

#define MLX5E_DECAP_TABLE_MISS_TAG 0xffff

struct mlx5e_decap_match {
	__be64	tun_id;
	__be32 src;
	__be32	dst;
	__u8 tos;
	__u8 ttl;
	__be16	tp_src;
	__be16	tp_dst;
	u32 mark;
	struct net_device *vxlan_device;
	struct mlx5_flow_handle *rule;
	int ref_count;
};

struct mlx5e_decap_match_table {
	struct mutex lock;
	struct notifier_block netevent_nb;
	struct mlx5e_decap_match data[];
};

void mlx5e_set_ttc_basic_params(struct mlx5e_priv *priv, struct ttc_params *ttc_params);
void mlx5e_set_ttc_ft_params(struct ttc_params *ttc_params);
void mlx5e_set_inner_ttc_ft_params(struct ttc_params *ttc_params);

int mlx5e_create_ttc_table(struct mlx5e_priv *priv, struct ttc_params *params,
			   struct mlx5e_ttc_table *ttc);
void mlx5e_destroy_ttc_table(struct mlx5e_priv *priv,
			     struct mlx5e_ttc_table *ttc);

int mlx5e_create_inner_ttc_table(struct mlx5e_priv *priv, struct ttc_params *params,
				 struct mlx5e_ttc_table *ttc);
void mlx5e_destroy_inner_ttc_table(struct mlx5e_priv *priv,
				   struct mlx5e_ttc_table *ttc);

void mlx5e_destroy_flow_table(struct mlx5e_flow_table *ft);

void mlx5e_enable_cvlan_filter(struct mlx5e_priv *priv);
void mlx5e_disable_cvlan_filter(struct mlx5e_priv *priv);

int mlx5e_sniffer_start(struct mlx5e_priv *priv);
int mlx5e_sniffer_stop(struct mlx5e_priv *priv);
int mlx5e_create_flow_steering(struct mlx5e_priv *priv);
void mlx5e_destroy_flow_steering(struct mlx5e_priv *priv);

int mlx5e_create_tx_steering(struct mlx5e_priv *priv);
void mlx5e_destroy_tx_steering(struct mlx5e_priv *priv);

int mlx5e_init_decap_matches_table(struct mlx5e_priv *priv);
void mlx5e_destroy_decap_matches_table(struct mlx5e_priv *priv);

int toggle_accelerated_decap(struct mlx5e_priv *priv, bool enable);

#endif /* __MLX5E_FLOW_STEER_H__ */

