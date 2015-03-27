var mongoose = require('mongoose');
var commentSchema = new mongoose.Schema({
	body: String,
	upvotes: {
		type: Number,
		default: 0
	},
	post: {
		type: mongoose.Schema.Types.ObjectId,
		ref: 'Post'
	}
});

commentSchema.methods.upvote = function(cb) {
	this.upvotes++;
	this.save(cb);
}

mongoose.model('Comment', commentSchema);